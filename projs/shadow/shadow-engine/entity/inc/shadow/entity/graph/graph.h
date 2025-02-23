#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include <functional>
#include <span>

#include "shadow/SHObject.h"
#include "shadow/entity/NodeManager.h"
#include "shadow/entity/NodeContainer.h"

namespace SH::Entities {

    typedef int RtmUuid;

    constexpr RtmUuid INVALID_UID = -1;

    namespace Debugger {
        class AllocationDebugger;
    }
    class NodeBase;

    class Node;

    class Actor;

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
    class API NodeBase : public SHObject {
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

        rtm_ptr<Node> parent;
        rtm_ptr<Scene> m_scene;
        World *m_world;
      public:
        template<class T> friend
        class rtm_ptr;

        friend class NodeManager;

        virtual ~NodeBase() {};

        void SetParent(const rtm_ptr<Node> &parent);

        rtm_ptr<Node> GetParent() const { return parent; }

        void SetScene(const rtm_ptr<Scene> &scene);

        void SetWorld(World *world);

        void Destroy();
    };

//###########################################################
//#################### Leaf nodes ###########################
//###########################################################

    // TODO: I don't think there can be any other types of leaf nodes than components, this is only here to make the
    //  inheritance names better
    class API LeafNode : public NodeBase {
      SHObject_Base(LeafNode)
    };

    class API Component : public LeafNode {
      SHObject_Base(Component)
    };


//###########################################################
//#################### Complex nodes ########################
//###########################################################

    // TODO: same with these the only difference is that Actors have a name as well.
    //  These two can be merged together if no use is found for unnamed full nodes

    template<typename T>
    concept IsActor = std::derived_from<T, Actor>;

    class API Node : public NodeBase {
      SHObject_Base(Node)
        std::vector<rtm_ptr<NodeBase>> hierarchy;
        std::vector<rtm_ptr<NodeBase>> internal_hierarchy;

      public:
        std::vector<rtm_ptr<NodeBase>> &GetHierarchy() { return hierarchy; }

        void AddChild(const rtm_ptr<NodeBase> &child, bool internal = false);

        template<class T>
        requires (not IsActor<T>)
        rtm_ptr<T> Add(const T &node, bool internal = false);

        template<class T>
        requires (IsActor<T>)
        rtm_ptr<T> Add(const T &node, bool internal = false);

        void RemoveChild(const rtm_ptr<NodeBase> &child, bool internal = false);
    };

    class API Actor : public Node {
      SHObject_Base(Actor)
      protected:
        /**
         * The name of this actor
         */
        std::string name;

      public:
        virtual void Build() = 0;

        const std::string& GetName() const { return name; }

        void SetName(std::string name) { this->name = name; }
    };

    class API Entity : public Node {
      SHObject_Base(Entity)
    };

    class API Scene : public Actor {
      SHObject_Base(Scene)
        std::vector<rtm_ptr<NodeBase>> static_hierarchy;
      public:
        Scene(std::string name) : Actor() { this->name = name; }

        void Build() override {};

        std::vector<rtm_ptr<NodeBase>> &GetStaticHierarchy() { return static_hierarchy; }
    };

    /**
     * It is responsible for the allocation of nodes and does not care about the graph of them
     * @brief Manages the memory and IDs of entities
     */
    class API NodeManager {
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
         * @brief Returns the correct container for the entity type.
         * Does not create a new one if it does not exist
         * @param typeID The type ID of the entity
         * @return The entity container accosted with this type
         */
        INodeContainer *GetNodeContainer(int typeID);

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

        template<typename T>
        rtm_ptr<T> TakeNode(const T &node) {
            // acquire memory for new entity object of type Type
            void *pObjectMemory = GetNodeContainer<T>()->allocate();

            new(pObjectMemory)T(node);

            //Assign the index and the UID to the object
            int runtimeIndex = this->AssignIndexToNode((T *) pObjectMemory);
            ((T *) pObjectMemory)->m_runtime_index = runtimeIndex;
            ((T *) pObjectMemory)->m_runtime_uid = nextUID;
            nextUID++;

            return rtm_ptr((T *) pObjectMemory);
        }

      public:
        friend class SH::Entities::Debugger::AllocationDebugger;

        NodeManager();

        template<class T>
        rtm_ptr<T> Add(const T &node) {
            rtm_ptr<T> ptr = TakeNode(node);
            return ptr;
        }

        /**
         * @brief Instantiates a new entity
         * @tparam T Type of the Entity
         * @tparam ARGS Constructor parameters of the Entity
         * @param args Constructor parameters of the Entity
         * @return
         * @obsolete
         */
        template<typename T, class ...ARGS>
        rtm_ptr<T> ConstructNode(ARGS &&... args) {
            //The type ID of the Entity we are trying to add
            const int CTID = T::TypeId();

            // acquire memory for new entity object of type Type
            void *pObjectMemory = GetNodeContainer<T>()->allocate();

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

        template<class T>
        requires std::is_base_of<NodeBase, T>::value
        void DestroyNode(T *node) {
            DestroyNode(node->m_runtime_index, node->GetTypeId());
        }

        template<class T>
        void DestroyNode(rtm_ptr<T> node) {
            DestroyNode(node->m_runtime_index, node->GetTypeId());
        }

        template<class T>
        inline T *GetEntityByIndex(int index) {
            return this->m_NodeLUT[index];
        }

        template<class T>
        inline NodeContainer<T> *GetContainerByType() {
            int CID = T::TypeId();

            auto it = this->m_NodeContainerRegistry.find(CID);

            if (it == this->m_NodeContainerRegistry.end())
                return nullptr;

            return static_cast<NodeContainer<T> *>(it->second);
        }
    };

    class API SystemBase {
      public:
        //SystemBase() {}
        //virtual ~SystemBase() = default;

        virtual void run(NodeManager &nmgr) = 0;
    };

    template<class Target> requires std::derived_from<Target, NodeBase>
    class API System : public SystemBase {
        std::function<void(Target &node)> m_Func;

      public:
        System<Target> &forEach(std::function<void(Target &)> func) {
            this->m_Func = func;
            return *this;
        }

        void run(NodeManager &nmgr) override {
            auto &container = *nmgr.GetContainerByType<Target>();

            for (auto it = container.begin(), end = container.end(); it != end; ++it) {
                m_Func(*it);
            }
        }
    };

    class API SystemManager {
        NodeManager &nodeManager;

        //vector storing the systems
        std::vector<std::shared_ptr<SystemBase>> m_Systems;

      public:
        SystemManager(NodeManager &nmgr) : nodeManager(nmgr) {

        }

        template<class T>
        std::shared_ptr<System<T>> system() {
            auto ptr = std::make_unique<System<T>>();
            m_Systems.push_back(std::make_shared<System<T>>());
            return std::dynamic_pointer_cast<System<T>>(m_Systems.back());
        }

        void run() {
            for (auto &s : m_Systems) {
                s->run(nodeManager);
            }
        }
    };

    class API RootNode : public Node {
      SHObject_Base(RootNode)
    };

    class API World : SHObject {
      SHObject_Base(World)
        rtm_ptr<RootNode> root;

        NodeManager manager;

        SystemManager systemManager;
      public:
        World();

        template<class T>
        requires std::derived_from<T, NodeBase>
        rtm_ptr<T> Add(const T &node) {
            auto ptr = manager.Add(node);
            ptr->SetWorld(this);
            return ptr;
        }

        template<class T>
        requires std::derived_from<T, Scene>
        rtm_ptr<T> AddScene(const T &scene) {
            return root->Add<T>(scene);
        }

        template<class T>
        std::shared_ptr<System<T>> system() {
            return systemManager.system<T>();
        }

        void Step() {
            systemManager.run();
        }

        NodeManager &GetManager() { return manager; }

        rtm_ptr<RootNode> GetRoot() { return root; }

        void Destroy(NodeBase *node);
    };

    template<class T>
    requires (not IsActor<T>)
    rtm_ptr<T> Node::Add(const T &node, bool internal) {
        auto ptr = m_world->Add(node);

        this->AddChild(ptr, internal);

        return ptr;
    }

    template<class T>
    requires (IsActor<T>)
    rtm_ptr<T> Node::Add(const T &node, bool internal) {
        auto ptr = m_world->Add<T>(node);

        this->AddChild(ptr, internal);

        if (Actor *h_actor = dynamic_cast<Actor *>(ptr.Get()))
            ptr->Build();

        return ptr;
    }

}