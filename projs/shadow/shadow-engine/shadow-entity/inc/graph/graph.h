#pragma once

#include <memory>
#include <vector>

#include "SHObject.h"

namespace ShadowEngine::Entities {

    using Rtm_UUID = int;

    constexpr Rtm_UUID INVALID_UID = -1;

/**
 * Runtime pointer to an Entity
 * It tracks the UUID of the linked entity
 * @tparam Type
 */
    template<class Type>
    class rtm_ptr {
    private:
        Type *m_ptr;

        Rtm_UUID m_uid;

    public:
        rtm_ptr(Type *ptr) : m_ptr(ptr), m_uid(ptr->m_runtimeUID) {}

        rtm_ptr() : m_ptr(nullptr) {}

        template<class T>
        explicit rtm_ptr(const rtm_ptr<T> &o) {
            m_ptr = (Type *) o.getInternalPointer();
            m_uid = o.getInternalUID();
        }

        Type *operator->() const {
            if (m_ptr->m_runtimeUID != m_uid) {
                assert(m_ptr->m_runtimeUID == m_uid);
                return nullptr;
            }
            return ((Type *) m_ptr);
        }

        bool isValid() const { return m_ptr != nullptr && m_ptr->m_runtimeUID == m_uid; }

        inline operator bool() const { return this->isValid(); }

        template<class T>
        inline bool operator==(rtm_ptr<T> o) const {
            return m_ptr == o.m_ptr &&
                   m_uid == o.m_uid;
        }

        template<class T>
        inline operator rtm_ptr<T>() const {
            return rtm_ptr<T>(m_ptr);
        }

        void setNull() {
            m_ptr = nullptr;
            m_uid = -1;
        }

        void *getInternalPointer() const {
            return (void *) m_ptr;
        }

        int getInternalUID() const { return m_uid; }

    };


    class Node;


/**
 * The base class for all things in the scene graph
 */
    class NodeBase : SHObject {
    SHObject_Base(NodeBase)

        /**
         * <summary>
         * This is the Globally unique ID of this Entity
         * </summary>
         * This ID will be only assigned to this Entity instance
         * It can be used to look up entities, but it is not recommended as it is a slow process
         * For Entity Lookup use the m_runtimeIndex
         */
        Rtm_UUID m_runtimeUID;

        int m_runtimeIndex;

        rtm_ptr<Node> parent;

    protected:
        NodeBase() {};

    public:
        virtual ~NodeBase() {};
    };

//###########################################################
//#################### Leaf nodes ###########################
//###########################################################

    class LeafNode : public NodeBase {

    };

    class Component : public LeafNode {

    };


//###########################################################
//#################### Complex nodes ########################
//###########################################################

    class Node : public NodeBase {
        std::vector<rtm_ptr<NodeBase>> hierarchy;
        std::vector<rtm_ptr<NodeBase>> internal_hierarchy;


    };


    class Actor : public Node {
        /**
         * The name of this actor
         */
        std::string name;
    };

    class Scene : public Node {
        std::vector<rtm_ptr<NodeBase>> static_hierarchy;
    };

}