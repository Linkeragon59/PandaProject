#pragma once

#include <map>
#include <set>
#include <functional>

#define DECLARE_NODE(NodeClass) \
public: \
const char* GetName() const override { return GetStaticName(); } \
private: \
friend class NodeRegister; \
static NodeClass* CreateInstance() { return new NodeClass(); } \
static const char* GetStaticName() { return #NodeClass; } \
NodeClass()

namespace GameCore
{
	class Node
	{
	public:
		virtual ~Node() {}
		virtual const char* GetName() const = 0;

		uint GetId() const { return myId; }

		struct Slot
		{
			Node* myNode = nullptr;
			uint myId = UINT_MAX;
		};
		void Connect(uint aSlotId, const Slot& anOutputSlot);
		void Disconnect(uint aSlotId);

		uint GetInputSlotsCount() const { return (uint)myInputSlots.size(); }
		uint GetOutputSlotsCount() const { return (uint)myOutputSlots.size(); }
		uint GetInputSlotId(uint aSlotIndex) const { return myInputSlots[aSlotIndex]; }
		uint GetOutputSlotId(uint aSlotIndex) const { return myOutputSlots[aSlotIndex]; }
		const std::map<uint, Slot>& GetInputs() const { return myInputs; }

	protected:
		friend class Graph;

		void AddInputSlot(uint aSlotId) { myInputSlots.push_back(aSlotId); }
		void AddOutputSlot(uint aSlotId) { myOutputSlots.push_back(aSlotId); }

		std::vector<uint> myInputSlots;
		std::vector<uint> myOutputSlots;
		std::map<uint, Slot> myInputs;
		std::set<uint> myOutputs; // We just want to know if the output is connected to something, so using a set

		uint myId = UINT_MAX;
	};

	class TestNode : public Node
	{
		DECLARE_NODE(TestNode)
		{
			AddInputSlot(0);
			AddInputSlot(2);
			AddInputSlot(4);
			AddOutputSlot(10);
			AddOutputSlot(20);
		}
	};
	
	class TestNode2 : public Node
	{
		DECLARE_NODE(TestNode2)
		{
			AddInputSlot(0);
			AddOutputSlot(0);
		}
	};

	class Graph
	{
	public:
		~Graph();

		const std::map<uint, Node*>& GetNodes() { return myNodes; }
		Node* GetNode(uint aNodeId) const { return myNodes.at(aNodeId); }

		uint AddNode(const char* aNodeName);
		void RemoveNode(uint aNodeId);

		void AddConnection(const Node::Slot& anInputSlot, const Node::Slot& anOutputSlot);
		void RemoveConnectionByInput(const Node::Slot& anInputSlot);
		void RemoveConnectionByOutput(const Node::Slot& anOutputSlot);

	private:
		uint GenerateNodeId() { return myNextNodeId++; }
		uint myNextNodeId = 0;

		std::map<uint, Node*> myNodes;
		std::map<uint, const Node*> myRootNodes;
	};

	class NodeRegister
	{
	public:
		NodeRegister();
		void RegisterNode(const char* aNodeName, const std::function<Node* ()>& aNodeCreateCallback);
		Node* CreateNode(const char* aNodeName);
		const std::set<std::string>& GetAvailableNodes() const { return myAvailableNodes; }

	private:
		std::hash<std::string_view> myHasher;
		std::map<size_t, std::function<Node* ()>> myNodeCreaters;
		std::set<std::string> myAvailableNodes; // Only for the editor to display the list
	};
}
