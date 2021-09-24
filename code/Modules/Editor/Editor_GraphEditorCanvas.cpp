#include "Editor_GraphEditorCanvas.h"

#include "GameWork_Graph.h"

#include "imgui_internal.h"

namespace Editor
{
	namespace
	{
		constexpr float locGraphMaxSizeFactor = 10.0f;

		constexpr float locZoomSpeed = 0.2f;
		constexpr float locMinZoomFactor = 0.5f;
		constexpr float locMaxZoomFactor = 5.0f;

		constexpr float locGridSize = 64.0f;
		constexpr ImU32 locGridColor = IM_COL32(200, 200, 200, 5);

		constexpr float locNodeSlotRadius = 5.0f;
		constexpr float locNodeSlotPadding = 1.0f;
		constexpr float locNodeCornerRounding = 1.5f;
		constexpr ImU32 locNodeColor = IM_COL32(150, 150, 150, 200);
		constexpr ImU32 locNodeSelectedColor = IM_COL32(255, 255, 150, 200);
		constexpr ImU32 locNodeHeaderColor = IM_COL32(255, 0, 100, 255);
		constexpr ImU32 locNodeSlotColor = IM_COL32(255, 255, 255, 255);
		constexpr ImU32 locNodeConnectionColor = IM_COL32(255, 255, 255, 255);
		constexpr ImU32 locNodeSelectionColor = IM_COL32(0, 120, 215, 50);

		bool locIsPointInRect(const ImVec2& aPoint, const ImVec2& aRectCorner, const ImVec2& aRectOppositeCorner)
		{
			const bool x = aRectCorner.x < aRectOppositeCorner.x ?
				aPoint.x >= aRectCorner.x && aPoint.x <= aRectOppositeCorner.x :
				aPoint.x >= aRectOppositeCorner.x && aPoint.x <= aRectCorner.x;
			const bool y = aRectCorner.y < aRectOppositeCorner.y ?
				aPoint.y >= aRectCorner.y && aPoint.y <= aRectOppositeCorner.y :
				aPoint.y >= aRectOppositeCorner.y && aPoint.y <= aRectCorner.y;
			return x && y;
		}

		bool locIsPointInCircle(const ImVec2& aPoint, const ImVec2& aCircleCenter, float aRadius)
		{
			ImVec2 vec = aCircleCenter - aPoint;
			return vec.x * vec.x + vec.y * vec.y < aRadius * aRadius;
		}
	}

	GraphEditorCanvas::~GraphEditorCanvas()
	{
		if (myGraph)
			delete myGraph;
	}

	void GraphEditorCanvas::Draw(const ImVec2& aPos, const ImVec2& aSize)
	{
		// Update Position and Size
		if (aPos != myPos || aSize != mySize)
		{
			myPos = aPos;
			mySize = aSize;
			if (myGraphPos == ImVec2())
				myGraphPos = mySize / 2;
			ClampGraphPos();
		}

		// Check control inputs
		ImGuiIO& io = ImGui::GetIO();
		ImGui::InvisibleButton("canvasButton", mySize, ImGuiButtonFlags_MouseButtonMask_);
		myIsHovered = ImGui::IsItemHovered();
		myIsActive = ImGui::IsItemActive();
		if (myIsHovered)
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
			{
				myGraphPos += io.MouseDelta;
				ClampGraphPos();
			}
			if (io.MouseWheel != 0.0f)
			{
				const float zoomMultiplicator = 1.0f + io.MouseWheel * locZoomSpeed;
				const float newZoomFactor = std::clamp(myZoomFactor * zoomMultiplicator,
					std::max(locMinZoomFactor, 1.0f / locGraphMaxSizeFactor),
					locMaxZoomFactor);
				if (myZoomFactor != newZoomFactor)
				{
					const ImVec2 mousePos(io.MousePos.x - myPos.x, io.MousePos.y - myPos.y);
					const ImVec2 mouseToCenter = myGraphPos - mousePos;
					myGraphPos = mousePos + mouseToCenter * (newZoomFactor / myZoomFactor);
					ClampGraphPos();
					myZoomFactor = newZoomFactor;
				}
			}
		}

		// Draw Grid
		const float gridSize = locGridSize * myZoomFactor;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (float x = fmodf(myGraphPos.x, gridSize); x < mySize.x; x += gridSize)
			draw_list->AddLine(ImVec2(myPos.x + x, myPos.y), ImVec2(myPos.x + x, myPos.y + mySize.y), locGridColor);
		for (float y = fmodf(myGraphPos.y, gridSize); y < mySize.y; y += gridSize)
			draw_list->AddLine(ImVec2(myPos.x, myPos.y + y), ImVec2(myPos.x + mySize.x, myPos.y + y), locGridColor);
		draw_list->AddCircleFilled(GraphPosToWindowPos(ImVec2()), 3.0f, locGridColor);

		// Draw Graph
		// --- TEMP ---
		if (myGraph == nullptr)
		{
			myGraph = new GameWork::TestGraph();
			myNodesDrawInfo[0].Update(myGraph->myNodes.at(0), ImVec2(100.0f, 0.0f), myZoomFactor);
			myNodesDrawInfo[1].Update(myGraph->myNodes.at(1), ImVec2(0.0f, -100.0f), myZoomFactor);
			myNodesDrawInfo[2].Update(myGraph->myNodes.at(2), ImVec2(0.0f, 100.0f), myZoomFactor);
			myNodesDrawInfo[3].Update(myGraph->myNodes.at(3), ImVec2(-100.0f, 100.0f), myZoomFactor);
		}
		// --- TEMP ---
		if (myGraph)
		{
			ImGui::SetWindowFontScale(myZoomFactor);
			Update();
			DrawNodes();
			DrawConnections();
			DrawSelectingRect();
			DrawOnGoingConnection();
			ImGui::SetWindowFontScale(1.0f);
		}
	}

	void GraphEditorCanvas::ClampGraphPos()
	{
		const ImVec2 graphMaxSize = locGraphMaxSizeFactor * myZoomFactor * mySize;
		const ImVec2 minPos = mySize - graphMaxSize / 2.0f;
		const ImVec2 maxPos = graphMaxSize / 2.0f;
		myGraphPos = Clamp(myGraphPos, minPos, maxPos);
	}

	ImVec2 GraphEditorCanvas::GraphPosToCanvasPos(const ImVec2& aGraphPos) const
	{
		return aGraphPos * myZoomFactor + myGraphPos;
	}

	ImVec2 GraphEditorCanvas::GraphPosToWindowPos(const ImVec2& aGraphPos) const
	{
		return aGraphPos * myZoomFactor + myGraphPos + myPos;
	}

	ImVec2 GraphEditorCanvas::CanvasPosToGraphPos(const ImVec2& aCanvasPos) const
	{
		return (aCanvasPos - myGraphPos) / myZoomFactor;
	}

	ImVec2 GraphEditorCanvas::CanvasPosToWindowPos(const ImVec2& aCanvasPos) const
	{
		return aCanvasPos + myPos;
	}

	ImVec2 GraphEditorCanvas::WindowPosToGraphPos(const ImVec2& aWindowPos) const
	{
		return (aWindowPos - myPos - myGraphPos) / myZoomFactor;
	}

	ImVec2 GraphEditorCanvas::WindowPosToCanvasPos(const ImVec2& aWindowPos) const
	{
		return aWindowPos - myPos;
	}

	void GraphEditorCanvas::GetHoveredItem(uint& aHoveredNode, uint& aHoveredNodeSlot, bool& aHoveredNodeSlotInput) const
	{
		const ImVec2 mousePos = WindowPosToGraphPos(ImGui::GetIO().MousePos);

		aHoveredNode = UINT_MAX;
		aHoveredNodeSlot = UINT_MAX;
		aHoveredNodeSlotInput = true;

		for (const auto& node : myGraph->myNodes)
		{
			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->myId);

			for (const auto& input : drawInfo.myInputSlotsPos)
			{
				if (locIsPointInCircle(mousePos, input.second, locNodeSlotRadius))
				{
					aHoveredNode = node.second->myId;
					aHoveredNodeSlot = input.first;
					aHoveredNodeSlotInput = true;
					return;
				}
			}

			for (const auto& output : drawInfo.myOutputSlotsPos)
			{
				if (locIsPointInCircle(mousePos, output.second, locNodeSlotRadius))
				{
					aHoveredNode = node.second->myId;
					aHoveredNodeSlot = output.first;
					aHoveredNodeSlotInput = false;
					return;
				}
			}

			if (locIsPointInRect(mousePos, drawInfo.myTopLeft, drawInfo.myBottomRight))
			{
				aHoveredNode = node.second->myId;
				return;
			}
		}
	}

	void GraphEditorCanvas::Update()
	{
		UpdateRectSelection();
		UpdateDraggedConnection();
		
		if (myIsHovered)
		{
			HandleKeyShorcuts();
			HandleNodeAdding();
			HandleNodeSelecting();
			HandleNodeMoving();
			HandleConnectionRemoving();
		}
	}

	void GraphEditorCanvas::UpdateRectSelection()
	{
		if (IsRectSelecting())
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				myInRectSelectionNodes.clear();
				const ImVec2 mousePos = WindowPosToGraphPos(ImGui::GetIO().MousePos);
				for (const auto& node : myGraph->myNodes)
				{
					CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->myId);
					if (locIsPointInRect(drawInfo.myCenter, myRectSelectionStart, mousePos))
					{
						myInRectSelectionNodes.insert(node.second->myId);
					}
				}
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				for (uint node : myInRectSelectionNodes)
				{
					if (mySelectedNodes.contains(node))
						mySelectedNodes.erase(node);
					else
						mySelectedNodes.insert(node);
				}
				myRectSelectionStart = ImVec2(FLT_MAX, FLT_MAX);
				myInRectSelectionNodes.clear();
			}
		}
	}

	void GraphEditorCanvas::UpdateDraggedConnection()
	{
		if (myConnectionStartSlot.IsValid())
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				uint nodeIndex = UINT_MAX;
				uint slotIndex = UINT_MAX;
				bool slotInput = true;
				GetHoveredItem(nodeIndex, slotIndex, slotInput);
				if (nodeIndex != UINT_MAX && slotIndex != UINT_MAX && nodeIndex != myConnectionStartSlot.myNodeIndex)
				{
					GameWork::Node::Slot startSlot = { myGraph->myNodes.at(myConnectionStartSlot.myNodeIndex), myConnectionStartSlot.mySlotIndex };
					GameWork::Node::Slot endSlot = { myGraph->myNodes.at(nodeIndex), slotIndex };
					if (slotInput)
					{
						myGraph->AddConnection(endSlot, startSlot);
					}
					else
					{
						myGraph->AddConnection(startSlot, endSlot);
					}
				}
				myConnectionStartSlot = DraggedSlot();
			}
		}
	}

	void GraphEditorCanvas::HandleKeyShorcuts()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Select all nodes
		if (io.KeysDown[io.KeyMap[ImGuiKey_A]] && io.KeyMods & ImGuiKeyModFlags_Ctrl)
		{
			for (const auto& node : myGraph->myNodes)
			{
				mySelectedNodes.insert(node.second->myId);
			}
		}

		// Delete nodes
		if (io.KeysDown[io.KeyMap[ImGuiKey_Delete]] || io.KeysDown[io.KeyMap[ImGuiKey_Backspace]])
		{
			for (uint node : mySelectedNodes)
			{
				GameWork::Node* nodeToRemove = myGraph->myNodes.at(node);
				myGraph->RemoveNode(node);
				delete nodeToRemove;

				myNodesDrawInfo.erase(node);
			}
			mySelectedNodes.clear();
			myInRectSelectionNodes.clear();
		}
	}

	void GraphEditorCanvas::HandleNodeAdding()
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			GameWork::TestNode* newNode = new GameWork::TestNode();
			myGraph->AddNode(newNode);
			myNodesDrawInfo[newNode->myId].Update(newNode, WindowPosToGraphPos(ImGui::GetIO().MousePos), myZoomFactor);
		}
	}

	void GraphEditorCanvas::HandleNodeSelecting()
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			uint nodeIndex = UINT_MAX;
			uint slotIndex = UINT_MAX;
			bool slotInput = true;
			GetHoveredItem(nodeIndex, slotIndex, slotInput);
			if (nodeIndex != UINT_MAX && slotIndex != UINT_MAX)
			{
				myConnectionStartSlot.myNodeIndex = nodeIndex;
				myConnectionStartSlot.mySlotIndex = slotIndex;
				myConnectionStartSlot.myInput = slotInput;
			}

			if (myConnectionStartSlot.IsValid())
			{
				mySelectedNodes.clear();
			}
			else
			{
				ImGuiIO& io = ImGui::GetIO();

				if (!io.KeyCtrl && !mySelectedNodes.contains(nodeIndex))
				{
					mySelectedNodes.clear();
				}

				if (nodeIndex == UINT_MAX)
				{
					const ImVec2 mousePos = WindowPosToGraphPos(io.MousePos);
					myRectSelectionStart = mousePos;
				}
				else
				{
					if (io.KeyCtrl && mySelectedNodes.contains(nodeIndex))
						mySelectedNodes.erase(nodeIndex);
					else
						mySelectedNodes.insert(nodeIndex);
				}
			}
		}
	}

	void GraphEditorCanvas::HandleNodeMoving()
	{
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !IsRectSelecting())
			{
				for (const auto& node : mySelectedNodes)
				{
					CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node);
					const ImVec2 newPos = drawInfo.myCenter + ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f) / myZoomFactor;
					drawInfo.Update(myGraph->myNodes.at(node), newPos, myZoomFactor);
				}
			}
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		}
	}

	void GraphEditorCanvas::HandleConnectionRemoving()
	{
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			uint nodeIndex = UINT_MAX;
			uint slotIndex = UINT_MAX;
			bool slotInput = true;
			GetHoveredItem(nodeIndex, slotIndex, slotInput);
			if (nodeIndex != UINT_MAX && slotIndex != UINT_MAX)
			{
				GameWork::Node::Slot slot = { myGraph->myNodes.at(nodeIndex), slotIndex };
				if (slotInput)
					myGraph->RemoveConnectionByInput(slot);
				else
					myGraph->RemoveConnectionByOutput(slot);
			}
		}
	}

	void GraphEditorCanvas::DrawNodes()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (const auto& node : myGraph->myNodes)
		{
			bool isSelected = mySelectedNodes.contains(node.second->myId);
			if (myInRectSelectionNodes.contains(node.second->myId))
				isSelected = !isSelected;

			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->myId);
			draw_list->AddRectFilled(GraphPosToWindowPos(drawInfo.myTopLeft), GraphPosToWindowPos(drawInfo.myBottomRight), isSelected ? locNodeSelectedColor : locNodeColor, locNodeCornerRounding, ImDrawFlags_RoundCornersAll);
			draw_list->AddRectFilled(GraphPosToWindowPos(drawInfo.myTopLeft), GraphPosToWindowPos(drawInfo.myHeaderBottomRight), locNodeHeaderColor, locNodeCornerRounding, ImDrawFlags_RoundCornersTop);
			draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), GraphPosToWindowPos(drawInfo.myTopLeft), locNodeSlotColor, "Test Node");
			for (const auto& input : drawInfo.myInputSlotsPos)
			{
				draw_list->AddCircle(GraphPosToWindowPos(input.second), (locNodeSlotRadius - locNodeSlotPadding) * myZoomFactor, locNodeSlotColor);
			}
			for (const auto& output : drawInfo.myOutputSlotsPos)
			{
				draw_list->AddCircle(GraphPosToWindowPos(output.second), (locNodeSlotRadius - locNodeSlotPadding) * myZoomFactor, locNodeSlotColor);
			}
		}
	}

	void GraphEditorCanvas::DrawConnections()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (const auto& node : myGraph->myNodes)
		{
			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->myId);
			for (const auto& input : node.second->myInputs)
			{
				const CachedNodeDrawInfo& inputDrawInfo = myNodesDrawInfo.at(input.second.myNode->myId);
				const ImVec2& outputSlotPos = GraphPosToWindowPos(inputDrawInfo.myOutputSlotsPos.at(input.second.myId));
				const ImVec2& inputSlotPos = GraphPosToWindowPos(drawInfo.myInputSlotsPos.at(input.first));
				draw_list->AddLine(outputSlotPos, inputSlotPos, locNodeConnectionColor);
			}
		}
	}

	void GraphEditorCanvas::DrawSelectingRect()
	{
		if (IsRectSelecting())
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImGuiIO& io = ImGui::GetIO();
			draw_list->AddRectFilled(GraphPosToWindowPos(myRectSelectionStart), io.MousePos, locNodeSelectionColor);
		}
	}

	void GraphEditorCanvas::DrawOnGoingConnection()
	{
		if (myConnectionStartSlot.IsValid())
		{
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(myConnectionStartSlot.myNodeIndex);
			const ImVec2& slotPos = GraphPosToWindowPos(myConnectionStartSlot.myInput ?
				drawInfo.myInputSlotsPos.at(myConnectionStartSlot.mySlotIndex) :
				drawInfo.myOutputSlotsPos.at(myConnectionStartSlot.mySlotIndex));
			draw_list->AddLine(slotPos, io.MousePos, locNodeConnectionColor);
		}
	}

	void GraphEditorCanvas::CachedNodeDrawInfo::Update(const GameWork::Node* aNode, const ImVec2& aCenterPosition, float aZoomFactor)
	{
		myHeaderSize = ImGui::CalcTextSize("Test Node") / aZoomFactor;
		mySize = ImVec2(myHeaderSize.x, myHeaderSize.y + (aNode->myInputSlots.size() + aNode->myOutputSlots.size()) * locNodeSlotRadius * 2.0f);

		myCenter = aCenterPosition;
		myTopLeft = aCenterPosition - mySize / 2.0f;
		myBottomRight = aCenterPosition + mySize / 2.0f;
		myHeaderBottomRight = ImVec2(myBottomRight.x, myBottomRight.y - mySize.y + myHeaderSize.y);

		myInputSlotsPos.clear();
		for (uint i = 0, e = (uint)aNode->myInputSlots.size(); i < e; ++i)
		{
			myInputSlotsPos[aNode->myInputSlots[i]] = ImVec2(myTopLeft.x + locNodeSlotRadius, myTopLeft.y + myHeaderSize.y + i * locNodeSlotRadius * 2.0f + locNodeSlotRadius);
		}
		myOutputSlotsPos.clear();
		for (uint i = 0, e = (uint)aNode->myOutputSlots.size(); i < e; ++i)
		{
			myOutputSlotsPos[aNode->myOutputSlots[i]] = ImVec2(myBottomRight.x - locNodeSlotRadius, myBottomRight.y - (e - i) * locNodeSlotRadius * 2.0f + locNodeSlotRadius);
		}
	}
}
