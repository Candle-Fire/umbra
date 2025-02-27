#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include <functional>
#include <span>

#include "shadow/SHObject.h"

//#include "archetype.h"

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

  template<typename T>
  concept NodeBaseType = std::is_base_of<NodeBase, T>::value;

//###########################################################
//#################### Leaf nodes ###########################
//###########################################################

  class API Component : public NodeBase {
  SHObject_Base(Component)
  };

//###########################################################
//#################### Complex nodes ########################
//###########################################################

  // TODO: the only difference is that Actors have a name as well.
  //  These two can be merged together if no use is found for unnamed full nodes

  template<typename T>
  concept IsActor = std::derived_from<T, Actor>;

  class API Node : public NodeBase {
  SHObject_Base(Node)
    std::vector<rtm_ptr<NodeBase>> hierarchy;
    std::vector<rtm_ptr<NodeBase>> internal_hierarchy;

  public:
    template<NodeBaseType T>
    rtm_ptr<T> Add(const T &node, bool internal = false);

    std::vector<rtm_ptr<NodeBase>> &GetHierarchy() { return hierarchy; }

    void AddChild(const rtm_ptr<NodeBase> &child, bool internal = false);

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

    const std::string &GetName() const { return name; }

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

}