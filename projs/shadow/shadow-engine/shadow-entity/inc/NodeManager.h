#pragma once

#include <unordered_map>
#include <cassert>
#include <vector>

#include "NodeContainer.h"
#include "SHObject.h"
#include "graph/graph.h"

namespace ShadowEngine::Entities {

    template<class T>
    struct rtm_ptr;

    class Entity;

    class NodeManager;

/**
 * It is responsible for the allocation of nodes and does not care about the graph of them
 * @brief Manages the memory and IDs of entities
 */
    class NodeManager {

      private:
        //Map the runtime index of the entity to the container
        using NodeContainerRegistry = std::unordered_map<TypeID, INodeContainer *>;

        /**
         * @brief Map of the Entity Containers mapped to the entity type ID
         */
        NodeContainerRegistry m_NodeContainerRegistry;

        using NodeLookupTable = std::vector<NodeBase *>;

        /**
         * This table is used to get a Entity by it's runtime Index.
         * It is a fast lookup, but it is not unique.
         * If a Entity is freed up it's index will be given out to another Entity of the same type
         *
         * To use the table simply access the element at the index of the Entity.
         *
         * @brief Quick access Look Up Table of active entities
         *
         */
        NodeLookupTable m_NodeLUT;

        //Extra number of spaces to allocate in the LUT
        const int NODE_LUT_GROW = 2048;
        int LUTNextFree = 0;
        bool LUTFragm = false;
        std::vector<int> LUTFragmFree;

        /**
         * @brief The next assignable Unique ID
         */
        int nextUID = 0;

        /**
         * @brief Returns the correct container for the entity type,
         * creating a new one if it does not exist
         * @tparam T The type of the entity
         * @return The entity container accosted with this type
         */
        template<class T>
        requires std::is_base_of<NodeBase, T>::value
        inline NodeContainer<T> *GetNodeContainer() {
            int CID = T::TypeId();

            auto container = (NodeContainer<T> *) GetNodeContainer(CID);

            if (container == nullptr) {
                container = new NodeContainer<T>();
                m_NodeContainerRegistry[CID] = container;
            }

            return container;
        }

        /**
         * @brief Returns the correct container for the entity type.
         * Does not create a new one if it does not exist
         * @param typeID The type ID of the entity
         * @return The entity container accosted with this type
         */
        inline INodeContainer *GetNodeContainer(int typeID);

        /**
         * @brief Assigns the next free LUT index to this entity
         * @param component
         * @return
         */
        int AssignIndexToNode(NodeBase *component);

        /**
         * @brief Frees up the given index
         * @param id
         */
        void ReleaseIndex(int id);

      public:
        NodeManager();

        /**
         * @brief Instantiates a new entity
         * @tparam T Type of the Entity
         * @tparam ARGS Constructor parameters of the Entity
         * @param args Constructor parameters of the Entity
         * @return
         */
        template<typename T, class ...ARGS>
        rtm_ptr<T> MakeNode(ARGS &&... args) {
            //The type ID of the Entity we are trying to add
            const int CTID = T::TypeId();

            // acquire memory for new entity object of type Type
            void *pObjectMemory = GetNodeContainer<T>()->CreateObject();

            //Assign the index and the UID to the object
            int runtimeIndex = this->AssignIndexToNode((T *) pObjectMemory);
            ((T *) pObjectMemory)->m_runtime_index = runtimeIndex;
            ((T *) pObjectMemory)->m_runtime_uid = nextUID;
            nextUID++;

            // create Entity in place
            NodeBase *component = new(pObjectMemory)T(std::forward<ARGS>(args)...);

            return rtm_ptr((T *) component);
        }

        void DestroyNode(int node_index, int typeID);

        /**
         * @brief Destroys the node with the given index
         * @tparam T The type of the entity to remove
         * @param node_index The entity index to be removed
         */
        template<class T>
        void DestroyNode(const int node_index) {
            this->DestroyNode(node_index, T::TypeID());
        }

        template<class T>
        requires std::is_base_of<NodeBase, T>::value
        void DestroyNode(T *node) {
            RemoveEntity<T>(node->m_runtime_index);
        }

        template<class T>
        void RemoveEntity(rtm_ptr<T> entity) {
            this->DestroyNode(entity->m_runtime_index, entity->GetTypeId());
        }

        template<class T>
        inline T *GetEntityByIndex(int index) {
            return this->m_NodeLUT[index];
        }

        template<class T>
        inline NodeContainer<T> *GetContainerByType() {
            int CID = T::TypeId();

            auto it = this->m_NodeContainerRegistry.find(CID);

            return static_cast<NodeContainer<T> *>(it->second);
        }
    };

}