#include "GameWork_Graph.h"

namespace GameWork
{

	Node::Node()
	{
	}

	Node::~Node()
	{
	}

	void Node::Connect(uint aSlotId, const Slot& anOutputSlot)
	{
		Assert(std::find(myInputSlots.begin(), myInputSlots.end(), aSlotId) != myInputSlots.end());
		Assert(!myInputs.contains(aSlotId));
		Assert(!anOutputSlot.myNode->myOutputs.contains(anOutputSlot.myId));

		myInputs[aSlotId] = anOutputSlot;

		anOutputSlot.myNode->myOutputs.insert(anOutputSlot.myId);
	}

	void Node::Disconnect(uint anId)
	{
		if (myInputs.contains(anId))
		{
			const Slot& outputSlot = myInputs.at(anId);
			outputSlot.myNode->myOutputs.erase(outputSlot.myId);
			
			myInputs.erase(anId);
		}
	}

	Graph::Graph()
	{
	}

	Graph::~Graph()
	{
		for (const std::pair<uint, Node*>& node : myNodes)
			delete node.second;
	}

	void Graph::AddNode(Node* aNode)
	{
		Assert(aNode->myId == UINT_MAX);
		aNode->myId = GenerateNodeId();
		Assert(aNode->myId != UINT_MAX, "Too many nodes in the graph!");
		
		myNodes[aNode->myId] = aNode;
		// Nodes are roots as long as they don't have any output connection
		myRootNodes[aNode->myId] = aNode;
	}

	void Graph::RemoveNode(uint aNodeId)
	{
		Assert(myNodes.contains(aNodeId) && myNodes.at(aNodeId)->myId == aNodeId);
		Node* nodeToRemove = myNodes.at(aNodeId);

		// Clear node outputs
		{
			std::vector<Node::Slot> slotsToRemove;
			for (uint output : nodeToRemove->myOutputs)
			{
				slotsToRemove.push_back({ nodeToRemove, output });
			}
			for (const Node::Slot& slot : slotsToRemove)
			{
				RemoveConnectionByOutput(slot);
			}
		}

		// Clear node inputs
		{
			std::vector<Node::Slot> slotsToRemove;
			for (const std::pair<uint, Node::Slot>& input : nodeToRemove->myInputs)
			{
				slotsToRemove.push_back({ nodeToRemove, input.first });
			}
			for (const Node::Slot& slot : slotsToRemove)
			{
				RemoveConnectionByInput(slot);
			}
		}

		myNodes.erase(aNodeId);
		myRootNodes.erase(aNodeId);
	}

	void Graph::AddConnection(const Node::Slot& anInputSlot, const Node::Slot& anOutputSlot)
	{
		Assert(anInputSlot.myNode->myId != anOutputSlot.myNode->myId);
		RemoveConnectionByInput(anInputSlot);
		RemoveConnectionByOutput(anOutputSlot);

		anInputSlot.myNode->Connect(anInputSlot.myId, anOutputSlot);

		myRootNodes.erase(anOutputSlot.myNode->myId);
	}

	void Graph::RemoveConnectionByInput(const Node::Slot& anInputSlot)
	{
		if (!anInputSlot.myNode->myInputs.contains(anInputSlot.myId))
			return;

		const Node* inputNode = anInputSlot.myNode->myInputs.at(anInputSlot.myId).myNode;
		Assert(!inputNode->myOutputs.empty() && !myRootNodes.contains(inputNode->myId));

		anInputSlot.myNode->Disconnect(anInputSlot.myId);

		if (inputNode->myOutputs.empty())
		{
			myRootNodes[inputNode->myId] = inputNode;
		}
	}

	void Graph::RemoveConnectionByOutput(const Node::Slot& anOutputSlot)
	{
		if (!anOutputSlot.myNode->myOutputs.contains(anOutputSlot.myId))
			return;

		// We have to iterate all the nodes to find the other end of the connection.
		// This is because nodes know their input nodes, but not their output nodes.
		for (const std::pair<uint, Node*>& node : myNodes)
		{
			for (const std::pair<uint, Node::Slot>& input : node.second->myInputs)
			{
				if (input.second.myNode->myId == anOutputSlot.myNode->myId && input.second.myId == anOutputSlot.myId)
				{
					RemoveConnectionByInput({ node.second, input.first });
					return;
				}
			}
		}
	}

	TestNode::TestNode()
	{
		AddInputSlot(0);
		AddInputSlot(2);
		AddInputSlot(4);

		AddOutputSlot(10);
		AddOutputSlot(20);
	}

	TestGraph::TestGraph()
	{
		TestNode* node1 = new TestNode();
		TestNode* node2 = new TestNode();
		TestNode* node3 = new TestNode();
		TestNode* node4 = new TestNode();

		AddNode(node1);
		AddNode(node2);
		AddNode(node3);
		AddNode(node4);

		AddConnection({ node1, 0 }, { node2, 20 });
		AddConnection({ node1, 4 }, { node3, 10 });
		AddConnection({ node3, 2 }, { node4, 20 });
	}
}
