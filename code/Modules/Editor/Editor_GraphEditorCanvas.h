#pragma once

#include "Base_imgui.h"

#include <map>
#include <set>

namespace GameWork
{
	struct Node;
	struct Graph;
}

namespace Editor
{
	class GraphEditorCanvas
	{
	public:
		~GraphEditorCanvas();

		void Draw(const ImVec2& aPos, const ImVec2& aSize);

		void SetGraph(GameWork::Graph* aGraph) { myGraph = aGraph; }

	private:
		void ClampGraphPos();
		ImVec2 GraphPosToCanvasPos(const ImVec2& aGraphPos) const;
		ImVec2 GraphPosToWindowPos(const ImVec2& aGraphPos) const;
		ImVec2 CanvasPosToGraphPos(const ImVec2& aCanvasPos) const;
		ImVec2 CanvasPosToWindowPos(const ImVec2& aCanvasPos) const;
		ImVec2 WindowPosToGraphPos(const ImVec2& aWindowPos) const;
		ImVec2 WindowPosToCanvasPos(const ImVec2& aWindowPos) const;

		bool IsRectSelecting() const { return myRectSelectionStart != ImVec2(FLT_MAX, FLT_MAX); }
		void GetHoveredItem(uint& aHoveredNode, uint& aHoveredNodeSlot, bool& aHoveredNodeSlotInput) const;

		void Update();
		void UpdateRectSelection();
		void UpdateDraggedConnection();
		void HandleKeyShorcuts();
		void HandleNodeAdding();
		void HandleNodeSelecting();
		void HandleNodeMoving();
		void HandleConnectionRemoving();

		void DrawNodes();
		void DrawConnections();
		void DrawSelectingRect();
		void DrawOnGoingConnection();

		ImVec2 myPos = ImVec2(0.0f, 0.0f);
		ImVec2 mySize = ImVec2(0.0f, 0.0f);
		float myZoomFactor = 1.0f;

		// Position of the center point of the graph in canvas coordinates
		ImVec2 myGraphPos = ImVec2(0.0f, 0.0f);

		bool myIsHovered = false;
		bool myIsActive = false;
		std::set<uint> mySelectedNodes;
		ImVec2 myRectSelectionStart = ImVec2(FLT_MAX, FLT_MAX);
		std::set<uint> myInRectSelectionNodes;

		struct DraggedSlot
		{
			bool IsValid() const { return myNodeIndex != UINT_MAX || mySlotIndex != UINT_MAX; }
			uint myNodeIndex = UINT_MAX;
			uint mySlotIndex = UINT_MAX;
			bool myInput = true;
		};
		DraggedSlot myConnectionStartSlot = DraggedSlot();

		GameWork::Graph* myGraph = nullptr;

		struct CachedNodeDrawInfo
		{
			void Update(const GameWork::Node* aNode, const ImVec2& aCenterPosition, float aZoomFactor);
			
			ImVec2 myHeaderSize;
			ImVec2 mySize;
			
			ImVec2 myCenter;
			ImVec2 myTopLeft;
			ImVec2 myBottomRight;
			ImVec2 myHeaderBottomRight;

			std::map<uint, ImVec2> myInputSlotsPos;
			std::map<uint, ImVec2> myOutputSlotsPos;
		};
		std::map<uint, CachedNodeDrawInfo> myNodesDrawInfo;
	};
}
