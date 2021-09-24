#pragma once

#include <map>
#include <set>

namespace GameWork
{
	struct Node
	{
		Node();
		virtual ~Node();

		bool operator==(const Node& anOther) const { return myId == anOther.myId; }

		void AddInputSlot(uint aSlotId) { myInputSlots.push_back(aSlotId); }
		void AddOutputSlot(uint aSlotId) { myOutputSlots.push_back(aSlotId); }

		struct Slot
		{
			Node* myNode = nullptr;
			uint myId = UINT_MAX;
		};

		void Connect(uint aSlotId, const Slot& anOutputSlot);
		void Disconnect(uint aSlotId);

		std::vector<uint> myInputSlots;
		std::vector<uint> myOutputSlots;
		std::map<uint, Slot> myInputs;
		std::set<uint> myOutputs; // We just want to know if the output is connected to something, so using a set

		uint myId = UINT_MAX;
	};

	struct Graph
	{
		Graph();
		virtual ~Graph();

		void AddNode(Node* aNode);
		void RemoveNode(uint aNodeId);

		void AddConnection(const Node::Slot& anInputSlot, const Node::Slot& anOutputSlot);
		void RemoveConnectionByInput(const Node::Slot& anInputSlot);
		void RemoveConnectionByOutput(const Node::Slot& anOutputSlot);

		uint GenerateNodeId() { return myNextNodeId++; }

		std::map<uint, Node*> myNodes;
		std::map<uint, const Node*> myRootNodes;

		uint myNextNodeId = 0;
	};

	struct TestNode : Node
	{
		TestNode();
	};

	struct TestGraph : Graph
	{
		TestGraph();
	};
}
