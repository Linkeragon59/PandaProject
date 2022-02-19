#pragma once

#include "Base_imgui.h"

#include <map>
#include <set>

namespace GameCore
{
	class Node;
	class Graph;
}

namespace Editor
{
	class GraphEditorCanvas
	{
	public:
		~GraphEditorCanvas();

		void Draw(const ImVec2& aPos, const ImVec2& aSize);

		void SetGraph(GameCore::Graph* aGraph) { myGraph = aGraph; }

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

		void HandleEvents();
		void HandleEventsHovered();
		void HandleEventsAlways();

		void DrawNodes();
		void DrawConnections();
		void DrawSelectingRect();
		void DrawOnGoingConnection();

		void LoadGraph(const char* aPath);
		void SaveGraph(const char* aPath);

		ImVec2 myPos = ImVec2(0.0f, 0.0f);
		ImVec2 mySize = ImVec2(0.0f, 0.0f);
		float myZoomFactor = 1.0f;

		// Position of the center point of the graph in canvas coordinates
		ImVec2 myGraphPos = ImVec2(0.0f, 0.0f);

		bool myIsHovered = false;
		bool myIsActive = false;

		std::set<uint> mySelectedNodes;
		std::set<uint> myInRectSelectionNodes;
		ImVec2 myRectSelectionStart = ImVec2(FLT_MAX, FLT_MAX);
		ImVec2 myNodeToAddPos = ImVec2(FLT_MAX, FLT_MAX);

		struct DraggedSlot
		{
			bool IsValid() const { return myNodeIndex != UINT_MAX || mySlotIndex != UINT_MAX; }
			uint myNodeIndex = UINT_MAX;
			uint mySlotIndex = UINT_MAX;
			bool myInput = true;
		};
		DraggedSlot myConnectionStartSlot = DraggedSlot();

		GameCore::Graph* myGraph = nullptr;

		struct CachedNodeDrawInfo
		{
			void Update(const GameCore::Node* aNode, const ImVec2& aCenterPosition, float aZoomFactor);
			
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
		std::map<uint, ImVec2> myNodesDrawInfoToUpdate;
	};
}
