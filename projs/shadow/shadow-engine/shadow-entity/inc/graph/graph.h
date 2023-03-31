#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "SHObject.h"
#include "NodeManager.h"

namespace ShadowEngine::Entities {

    typedef int RtmUuid;

    constexpr RtmUuid INVALID_UID = -1;

    class NodeBase;

    class Node;

    class Scene;

    class World;

    /**
     * Runtime pointer to an Entity
     * It tracks the UUID of the linked entity
     * @tparam Type
     */
    template<class Type>
    class rtm_ptr {
      private:
        Type *m_ptr;

        RtmUuid m_uid;

      public:
        rtm_ptr(Type *ptr) : m_ptr(ptr), m_uid(ptr->m_runtime_uid) {}

        rtm_ptr() : m_ptr(nullptr) {}

        template<class T>
        explicit rtm_ptr(const rtm_ptr<T> &o) {
            m_ptr = (Type *) o.GetInternalPointer();
            m_uid = o.GetInternalUid();
        }

        Type *operator->() const {
            if (m_ptr->m_runtime_uid != m_uid) {
                assert(m_ptr->m_runtime_uid == m_uid);
                return nullptr;
            }
            return ((Type *) m_ptr);
        }

        bool IsValid() const { return m_ptr != nullptr && m_ptr->m_runtime_uid == m_uid; }

        inline operator bool() const { return this->IsValid(); }

        template<class T>
        inline bool operator==(rtm_ptr<T> o) const {
            return m_ptr == o.m_ptr &&
                m_uid == o.m_uid;
        }

        template<class T>
        inline operator rtm_ptr<T>() const {
            return rtm_ptr<T>(m_ptr);
        }

        void SetNull() {
            m_ptr = nullptr;
            m_uid = -1;
        }

        void *GetInternalPointer() const {
            return (void *) m_ptr;
        }

        NodeBase *GetAsNodeBase() const {
            return (NodeBase *) m_ptr;
        }

        Type *Get() const {
            return m_ptr;
        }

        int GetInternalUid() const { return m_uid; }

    };

    /**
     * The base class for all things in the scene graph
     */
    class NodeBase : public SHObject {
      SHObject_Base(NodeBase)

        /**
         * <summary>
         * This is the Globally unique ID of this Entity
         * </summary>
         * This ID will be only assigned to this Entity instance
         * It can be used to look up entities, but it is not recommended as it is a slow process
         * For Entity Lookup use the m_runtime_index
         */
        RtmUuid m_runtime_uid;

        /**
         * @brief The index of this entity in the Entity Look Up Table
         * This is a fast way to access the entity, but it is not unique
         * If an Entity is freed up it's index will be given out to another Entity of the same type
         */
        int m_runtime_index;

      protected:
        NodeBase() {};

        rtm_ptr<NodeBase> parent;
        rtm_ptr<Scene> m_scene;
        World *m_world;
      public:
        template<class T> friend
        class rtm_ptr;

        friend class NodeManager;

        virtual ~NodeBase() {};

        void SetParent(const rtm_ptr<NodeBase> &parent);

        void SetScene(const rtm_ptr<Scene> &scene);

        void SetWorld(World *world);
    };

//###########################################################
//#################### Leaf nodes ###########################
//###########################################################

    // TODO: I don't think there can be any other types of leaf nodes than components, this is only here to make the
    //  inheritance names better
    class LeafNode : public NodeBase {
      SHObject_Base(LeafNode)
    };

    class Component : public LeafNode {
      SHObject_Base(Component)
    };


//###########################################################
//#################### Complex nodes ########################
//###########################################################

    // TODO: same with these the only difference is that Actors have a name as well.
    //  These two can be merged together if no use is found for unnamed full nodes

    class Node : public NodeBase {
      SHObject_Base(Node)
        std::vector<rtm_ptr<NodeBase>> hierarchy;
        std::vector<rtm_ptr<NodeBase>> internal_hierarchy;

      public:
        std::vector<rtm_ptr<NodeBase>> &GetHierarchy() { return hierarchy; }

        void AddChild(const rtm_ptr<NodeBase> &child);

        template<class T>
        rtm_ptr<T> Add(const T &child);
    };

    class Actor : public Node {
      SHObject_Base(Actor)
        /**
         * The name of this actor
         */
        std::string name;

      public:
        virtual void Build() = 0;

        std::string GetName() const { return name; }

        void SetName(std::string name) { this->name = name; }
    };

    class Entity : public Node {
      SHObject_Base(Entity)
    };

    class Scene : public Node {
      SHObject_Base(Scene)
        std::vector<rtm_ptr<NodeBase>> static_hierarchy;
    };

    /**
     * It is responsible for the allocation of nodes and does not care about the graph of them
     * @brief Manages the memory and IDs of entities
     */
    class NodeManager {
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

        template<class T>
        rtm_ptr<T> Add(const T &node) {
            rtm_ptr<T> ptr = TakeNode(node);
            return ptr;
        }

        template<typename T>
        rtm_ptr<T> TakeNode(const T &node) {
            // acquire memory for new entity object of type Type
            void *pObjectMemory = GetNodeContainer<T>()->CreateObject();

            new(pObjectMemory)T(node);

            //Assign the index and the UID to the object
            int runtimeIndex = this->AssignIndexToNode((T *) pObjectMemory);
            ((T *) pObjectMemory)->m_runtime_index = runtimeIndex;
            ((T *) pObjectMemory)->m_runtime_uid = nextUID;
            nextUID++;

            return rtm_ptr((T *) pObjectMemory);
        }

        /**
         * @brief Instantiates a new entity
         * @tparam T Type of the Entity
         * @tparam ARGS Constructor parameters of the Entity
         * @param args Constructor parameters of the Entity
         * @return
         */
        template<typename T, class ...ARGS>
        rtm_ptr<T> ConstructNode(ARGS &&... args) {
            //The type ID of the Entity we are trying to add
            const int CTID = T::TypeId();

            // acquire memory for new entity object of type Type
            void *pObjectMemory = GetNodeContainer<T>()->CreateObject();

            // create Entity in place
            NodeBase *component = new(pObjectMemory)T(std::forward<ARGS>(args)...);

            //Assign the index and the UID to the object
            int runtimeIndex = this->AssignIndexToNode((T *) pObjectMemory);
            ((T *) pObjectMemory)->m_runtime_index = runtimeIndex;
            ((T *) pObjectMemory)->m_runtime_uid = nextUID;
            nextUID++;

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

    class World : SHObject {
      SHObject_Base(World)
        std::vector<rtm_ptr<Scene>> scenes;

        NodeManager manager;
      public:
        void AddScene(rtm_ptr<Scene> scene);

        std::vector<rtm_ptr<Scene>> GetScenes() const { return scenes; }

        NodeManager &GetManager() { return manager; }

        template<class T>
        requires std::derived_from<T, NodeBase>
        rtm_ptr<T> Add(const T &node, rtm_ptr<Node> parent = nullptr) {
            auto ptr = manager.Add(node);
            ptr->SetWorld(this);

            if (parent)
                parent->AddChild(ptr);

            return ptr;
        }
    };

    template<class T>
    rtm_ptr<T> Node::Add(const T &child) {
        return m_world->Add(child, this);
    }

}