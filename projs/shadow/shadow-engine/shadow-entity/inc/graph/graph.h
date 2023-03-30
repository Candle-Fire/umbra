#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "SHObject.h"

namespace ShadowEngine::Entities {

    typedef int RtmUuid;

    constexpr RtmUuid INVALID_UID = -1;

    class NodeBase;

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
      public:
        template<class T> friend
        class rtm_ptr;

        friend class NodeManager;

        virtual ~NodeBase() {};

        void SetParent(rtm_ptr<NodeBase> parent) {
            this->parent = parent;
        }
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

        /**
         * Adds a child to this node's hierarchy
         * @param child
         */
        void AddChild(rtm_ptr<NodeBase> child);

        /**
         * Adds a child to this node's internal hierarchy
         * @param child
         */
        void AddInternalChild(rtm_ptr<NodeBase> child);
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

    class Scene : public Node {
      SHObject_Base(Scene)
        std::vector<rtm_ptr<NodeBase>> static_hierarchy;
    };

    class RootNode : public NodeBase {
      SHObject_Base(RootNode)
        std::vector<rtm_ptr<Scene>> scenes;
      public:
        void AddScene(rtm_ptr<Scene> scene);

        std::vector<rtm_ptr<Scene>> &GetScenes() { return scenes; }
    };

}