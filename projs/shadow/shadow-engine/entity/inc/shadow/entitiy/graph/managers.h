#pragma once
#include "shadow/exports.h"
#include "shadow/SHObject.h"
#include "shadow/util/hash.h"

#include "shadow/entitiy/graph/nodes.h"
#include "shadow/entitiy/NodeContainer.h"

namespace SH::Entities {

  using ComponentTypeId = SH::TypeId;
  using EntityId = RtmUuid;

  // List of component types
  using Types = std::vector<ComponentTypeId>;
}

template<>
struct std::hash<SH::Entities::Types> {
std::size_t operator()(const SH::Entities::Types &s) const noexcept;
};


namespace SH::Entities{

  class API NodeManager{

    //##############################################
    //#############  Storage  ######################
    //##############################################

    //Map the runtime index of the entity to the container
    using NodeContainerRegistry = std::unordered_map<ComponentTypeId, INodeContainer *>;

    NodeContainerRegistry m_NodeContainerRegistry;

    //##############################################
    //#############  Archetypes  ###################
    //##############################################

    /**
     * @brief The next assignable Unique ID
     */
    int nextUID = 0;

    struct Archetype {
      // Unique id of an archetype
      using Id = std::uint32_t;
      // vector for a component type T
      using Column = std::vector<void *>;

      Id id;
      Types types;
      std::vector<Column> children;
    };
    Archetype::Id next_archetype_id = 0;

    std::vector<Archetype> archetype;

    struct ArchetypeRecord {
      size_t column;
    };
    using ArchetypeMap = std::unordered_map<Archetype::Id, ArchetypeRecord>;
    ///Maps each component type to every archetype it is part of
    std::unordered_map<ComponentTypeId, ArchetypeMap> component_index;

    struct Record {
    public:
      Archetype *archetype;
      size_t row;
    };
    ///Maps each entity to the archetype and index in the archetype
    std::unordered_map<EntityId, Record> entity_index;

    std::unordered_map<Types, Archetype*> archetype_index;

    void *get_component(const EntityId &entity, const ComponentTypeId &component);

    template<SH::Entities::NodeBaseType Component>
    void* get_component(const EntityId& entity);
    
    void add_component(const EntityId& entity, const NodeBase& child);

    const Archetype& get_archetype(const Types types);

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
    template<NodeBaseType T>
    inline NodeContainer <T> *GetNodeContainer() {
        int CID = T::TypeId();

        auto container = (NodeContainer <T> *) GetNodeContainer(CID);

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

    template<NodeBaseType T>
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
    void AddNode(const NodeBase &node, const EntityId parent = INVALID_UID ){
        auto newNode = this->TakeNode(node);
        if(parent != INVALID_UID){
            add_component(parent, node);
        }
    }
  };


  class API SystemBase {
  public:
    virtual void run(NodeManager &nmgr) = 0;
  };

  template<NodeBaseType Target>
  class API System : public SystemBase {
    std::function<void(Target &node)> m_Func;

  public:
    System<Target> &forEach(std::function<void(Target &)> func) {
        this->m_Func = func;
        return *this;
    }

    void run(NodeManager &nmgr) override {
        /*
        auto &container = *nmgr.GetContainerByType<Target>();

        for (
            auto it = container.begin(), end = container.end();
            it !=
                end;
            ++it) {
            m_Func(*it);
        }
        */
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
        auto ptr = std::make_unique<System<T >>();
        m_Systems.push_back(std::make_shared<System<T >>());
        return std::dynamic_pointer_cast<System<T >>(m_Systems.back());
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

    template<NodeBaseType T>
    rtm_ptr<T> Add(const T &node) {
        //auto ptr = manager.Add(node);
        //ptr->SetWorld(this);
        //return ptr;
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


  template<NodeBaseType T>
  rtm_ptr<T> Node::Add(const T &node, bool internal) {
      auto ptr = m_world->Add(node);

      this->AddChild(ptr, internal);

      return ptr;
  }

}