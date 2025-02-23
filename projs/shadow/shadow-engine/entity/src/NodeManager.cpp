
#include "shadow/entity/NodeManager.h"
#include "shadow/entity/graph/graph.h"

namespace SH::Entities {

    //NodeManager *NodeManager::Instance = nullptr;

    int NodeManager::AssignIndexToNode(NodeBase *component) {
        int i = 0;
        if (LUTFragm) {
            i = LUTFragmFree.back();
            LUTFragmFree.pop_back();
            if (LUTFragmFree.empty()) {
                LUTFragm = false;
            }
        } else {
            i = LUTNextFree;
            LUTNextFree++;
            if (!(i < m_NodeLUT.size())) {
                this->m_NodeLUT.resize(this->m_NodeLUT.size() + NODE_LUT_GROW, nullptr);
            }
        }

        this->m_NodeLUT[i] = component;
        return i;
    }

    void NodeManager::ReleaseIndex(int id) {
        assert(id < this->m_NodeLUT.size() && "Invalid component id");

        //If this free is from the middle of the LUT
        //We record that the LUT is fragmented
        if (id != this->m_NodeLUT.size() - 1) {
            LUTFragm = true;
            LUTFragmFree.push_back(id);
        }

        this->m_NodeLUT[id] = nullptr;
    }

    void NodeManager::DestroyNode(const int node_index, const int typeID) {
        //Lookup of the entity to be removed
        NodeBase *entity = this->m_NodeLUT[node_index];
        assert(entity != nullptr && "FATAL: Trying to remove a entity that doesn't exist");

        //Invalidate the UID
        entity->m_runtime_uid = INVALID_UID;
        // unmap entity id
        ReleaseIndex(node_index);

        entity->~NodeBase();

        // release object memory
        GetNodeContainer(typeID)->DestroyObject((void *) entity);
    }

    INodeContainer *NodeManager::GetNodeContainer(int typeID) {
        auto it = this->m_NodeContainerRegistry.find(typeID);
        INodeContainer *cc = nullptr;

        if (!(it == this->m_NodeContainerRegistry.end()))
            cc = static_cast<INodeContainer *>(it->second);

        return cc;
    }

    NodeManager::NodeManager() {
        //Instance = this;
    }

}