#include "Editor_GraphEditorCanvas.h"

/*

#include "GameCore_Facade.h"
#include "GameCore_Graph.h"

#include "imgui_internal.h"

#include <format>
#include <iostream>
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

		constexpr const char* locAddeNodeMenuId = "GraphEditorCanvas:AddNodeMenu";

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
			myGraph = new GameCore::Graph();
			myGraph->AddNode("TestNode");
			myGraph->AddNode("TestNode");
			myGraph->AddNode("TestNode");
			myGraph->AddNode("TestNode");
			myGraph->AddConnection({ myGraph->GetNode(0), 0 }, { myGraph->GetNode(1), 20 });
			myGraph->AddConnection({ myGraph->GetNode(0), 4 }, { myGraph->GetNode(2), 10 });
			myGraph->AddConnection({ myGraph->GetNode(2), 2 }, { myGraph->GetNode(3), 20 });
			myNodesDrawInfo[0].Update(myGraph->GetNode(0), ImVec2(100.0f, 0.0f), myZoomFactor);
			myNodesDrawInfo[1].Update(myGraph->GetNode(1), ImVec2(0.0f, -100.0f), myZoomFactor);
			myNodesDrawInfo[2].Update(myGraph->GetNode(2), ImVec2(0.0f, 100.0f), myZoomFactor);
			myNodesDrawInfo[3].Update(myGraph->GetNode(3), ImVec2(-100.0f, 100.0f), myZoomFactor);
		}
		// --- TEMP ---
		if (myGraph)
		{
			HandleEvents();
			DrawNodes();
			DrawConnections();
			DrawSelectingRect();
			DrawOnGoingConnection();
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

		for (const auto& node : myGraph->GetNodes())
		{
			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->GetId());

			for (const auto& input : drawInfo.myInputSlotsPos)
			{
				if (locIsPointInCircle(mousePos, input.second, locNodeSlotRadius))
				{
					aHoveredNode = node.second->GetId();
					aHoveredNodeSlot = input.first;
					aHoveredNodeSlotInput = true;
					return;
				}
			}

			for (const auto& output : drawInfo.myOutputSlotsPos)
			{
				if (locIsPointInCircle(mousePos, output.second, locNodeSlotRadius))
				{
					aHoveredNode = node.second->GetId();
					aHoveredNodeSlot = output.first;
					aHoveredNodeSlotInput = false;
					return;
				}
			}

			if (locIsPointInRect(mousePos, drawInfo.myTopLeft, drawInfo.myBottomRight))
			{
				aHoveredNode = node.second->GetId();
				return;
			}
		}
	}

	void GraphEditorCanvas::HandleEvents()
	{
		HandleEventsHovered();
		HandleEventsAlways();
	}

	void GraphEditorCanvas::HandleEventsHovered()
	{
		// Add to this function all the handling that should only be done when the canvas is hovered

		if (!myIsHovered)
			return;

		ImGuiIO& io = ImGui::GetIO();

		// Select all nodes
		if (io.KeysDown[io.KeyMap[ImGuiKey_A]] && io.KeyMods & ImGuiKeyModFlags_Ctrl)
		{
			for (const auto& node : myGraph->GetNodes())
			{
				mySelectedNodes.insert(node.second->GetId());
			}
		}

		// Delete selected nodes
		if (io.KeysDown[io.KeyMap[ImGuiKey_Delete]] || io.KeysDown[io.KeyMap[ImGuiKey_Backspace]])
		{
			for (uint node : mySelectedNodes)
			{
				myGraph->RemoveNode(node);
				myNodesDrawInfo.erase(node);
			}
			mySelectedNodes.clear();
			myInRectSelectionNodes.clear();
		}

		// Add connections, select nodes
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

		// Move nodes
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !IsRectSelecting())
			{
				const ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
				if (dragDelta.x != 0.0f || dragDelta.y != 0.0f)
				{
					for (const auto& node : mySelectedNodes)
					{
						myNodesDrawInfoToUpdate[node] = myNodesDrawInfo.at(node).myCenter + dragDelta / myZoomFactor;
					}
				}
			}
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
		}

		// Remove connections
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			uint nodeIndex = UINT_MAX;
			uint slotIndex = UINT_MAX;
			bool slotInput = true;
			GetHoveredItem(nodeIndex, slotIndex, slotInput);

			if (nodeIndex != UINT_MAX && slotIndex != UINT_MAX)
			{
				GameCore::Node::Slot slot = { myGraph->GetNode(nodeIndex), slotIndex };
				if (slotInput)
					myGraph->RemoveConnectionByInput(slot);
				else
					myGraph->RemoveConnectionByOutput(slot);
			}
		}

		// Add new nodes
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup(locAddeNodeMenuId);
			myNodeToAddPos = WindowPosToGraphPos(io.MousePos);
		}
	}

	void GraphEditorCanvas::HandleEventsAlways()
	{
		// Add to this function all the handling that can be done even when the canvas is not hovered

		// Update the rect selection
		if (IsRectSelecting())
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				myInRectSelectionNodes.clear();
				const ImVec2 mousePos = WindowPosToGraphPos(ImGui::GetIO().MousePos);
				for (const auto& node : myGraph->GetNodes())
				{
					CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->GetId());
					if (locIsPointInRect(drawInfo.myCenter, myRectSelectionStart, mousePos))
					{
						myInRectSelectionNodes.insert(node.second->GetId());
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

		// Update the on-going connection
		if (myConnectionStartSlot.IsValid() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			uint nodeIndex = UINT_MAX;
			uint slotIndex = UINT_MAX;
			bool slotInput = true;
			GetHoveredItem(nodeIndex, slotIndex, slotInput);

			if (nodeIndex != UINT_MAX && slotIndex != UINT_MAX && nodeIndex != myConnectionStartSlot.myNodeIndex)
			{
				GameCore::Node::Slot startSlot = { myGraph->GetNode(myConnectionStartSlot.myNodeIndex), myConnectionStartSlot.mySlotIndex };
				GameCore::Node::Slot endSlot = { myGraph->GetNode(nodeIndex), slotIndex };
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

		// Handle the popup menu to add nodes
		if (ImGui::BeginPopup(locAddeNodeMenuId))
		{
			ImGui::Text(" Nodes ");
			ImGui::Separator();
			for (const std::string& nodeName : GameCore::Facade::GetInstance()->GetNodeRegister()->GetAvailableNodes())
			{
				if (ImGui::Selectable(nodeName.c_str()))
				{
					uint newNodeId = myGraph->AddNode(nodeName.c_str());
					myNodesDrawInfoToUpdate[newNodeId] = myNodeToAddPos;
					myNodeToAddPos = ImVec2(FLT_MAX, FLT_MAX);
				}
			}
			ImGui::EndPopup();
		}
	}

	void GraphEditorCanvas::DrawNodes()
	{
		ImGui::SetWindowFontScale(myZoomFactor);

		for (const auto& nodeToUpdate : myNodesDrawInfoToUpdate)
		{
			// There can be new nodes here, so using operator[] on purpose
			myNodesDrawInfo[nodeToUpdate.first].Update(myGraph->GetNode(nodeToUpdate.first), nodeToUpdate.second, myZoomFactor);
		}
		myNodesDrawInfoToUpdate.clear();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (const auto& node : myGraph->GetNodes())
		{
			bool isSelected = mySelectedNodes.contains(node.second->GetId());
			if (myInRectSelectionNodes.contains(node.second->GetId()))
				isSelected = !isSelected;

			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->GetId());
			draw_list->AddRectFilled(GraphPosToWindowPos(drawInfo.myTopLeft), GraphPosToWindowPos(drawInfo.myBottomRight), isSelected ? locNodeSelectedColor : locNodeColor, locNodeCornerRounding, ImDrawFlags_RoundCornersAll);
			draw_list->AddRectFilled(GraphPosToWindowPos(drawInfo.myTopLeft), GraphPosToWindowPos(drawInfo.myHeaderBottomRight), locNodeHeaderColor, locNodeCornerRounding, ImDrawFlags_RoundCornersTop);
			draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), GraphPosToWindowPos(drawInfo.myTopLeft), locNodeSlotColor, node.second->GetName());
			for (const auto& input : drawInfo.myInputSlotsPos)
			{
				draw_list->AddCircle(GraphPosToWindowPos(input.second), (locNodeSlotRadius - locNodeSlotPadding) * myZoomFactor, locNodeSlotColor);
			}
			for (const auto& output : drawInfo.myOutputSlotsPos)
			{
				draw_list->AddCircle(GraphPosToWindowPos(output.second), (locNodeSlotRadius - locNodeSlotPadding) * myZoomFactor, locNodeSlotColor);
			}
		}

		ImGui::SetWindowFontScale(1.0f);
	}

	void GraphEditorCanvas::DrawConnections()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (const auto& node : myGraph->GetNodes())
		{
			const CachedNodeDrawInfo& drawInfo = myNodesDrawInfo.at(node.second->GetId());
			for (const auto& input : node.second->GetInputs())
			{
				const CachedNodeDrawInfo& inputDrawInfo = myNodesDrawInfo.at(input.second.myNode->GetId());
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

	void GraphEditorCanvas::LoadGraph(const char* / *aPath* /)
	{

	}

	void GraphEditorCanvas::SaveGraph(const char* / *aPath* /)
	{
		rapidjson::Document doc;
		doc.SetObject();
		rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = doc.GetAllocator();

		for (const auto& drawInfo : myNodesDrawInfo)
		{
			rapidjson::Value key(std::format("{}", drawInfo.first), allocator);
			rapidjson::Value value(std::format("{};{}", drawInfo.second.myCenter.x, drawInfo.second.myCenter.y), allocator);
			doc.AddMember(key, value, allocator);
		}

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		buffer.GetString();
		std::cout << buffer.GetString() << std::endl;
	}

	void GraphEditorCanvas::CachedNodeDrawInfo::Update(const GameCore::Node* aNode, const ImVec2& aCenterPosition, float aZoomFactor)
	{
		myHeaderSize = ImGui::CalcTextSize(aNode->GetName()) / aZoomFactor;
		mySize = ImVec2(myHeaderSize.x, myHeaderSize.y + (aNode->GetInputSlotsCount() + aNode->GetOutputSlotsCount()) * locNodeSlotRadius * 2.0f);

		myCenter = aCenterPosition;
		myTopLeft = aCenterPosition - mySize / 2.0f;
		myBottomRight = aCenterPosition + mySize / 2.0f;
		myHeaderBottomRight = ImVec2(myBottomRight.x, myBottomRight.y - mySize.y + myHeaderSize.y);

		myInputSlotsPos.clear();
		for (uint i = 0, e = aNode->GetInputSlotsCount(); i < e; ++i)
		{
			myInputSlotsPos[aNode->GetInputSlotId(i)] = ImVec2(myTopLeft.x + locNodeSlotRadius, myTopLeft.y + myHeaderSize.y + i * locNodeSlotRadius * 2.0f + locNodeSlotRadius);
		}
		myOutputSlotsPos.clear();
		for (uint i = 0, e = aNode->GetOutputSlotsCount(); i < e; ++i)
		{
			myOutputSlotsPos[aNode->GetOutputSlotId(i)] = ImVec2(myBottomRight.x - locNodeSlotRadius, myBottomRight.y - (e - i) * locNodeSlotRadius * 2.0f + locNodeSlotRadius);
		}
	}
}

*/
