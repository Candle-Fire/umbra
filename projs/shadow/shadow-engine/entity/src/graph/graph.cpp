#include "shadow/entity/graph/graph.h"

namespace SH::Entities {

    SHObject_Base_Impl(NodeBase)

    void NodeBase::SetParent(const rtm_ptr<Node> &parent) {
        this->parent = parent;
    }

    void NodeBase::SetScene(const rtm_ptr<Scene> &scene) {
        this->m_scene = scene;
    }

    void NodeBase::SetWorld(World *world) {
        this->m_world = world;
    }

    void NodeBase::Destroy() {
        this->m_world->Destroy(this);
    }

    SHObject_Base_Impl(LeafNode)

    SHObject_Base_Impl(Component)

    SHObject_Base_Impl(Node)

    void Node::AddChild(const rtm_ptr<NodeBase> &child, bool internal) {
        if (internal)
            this->internal_hierarchy.push_back(child);
        else
            this->hierarchy.push_back(child);
        // set the child's parent to this
        child->SetParent(this);
        // set the child's scene to this
        child->SetScene(this->m_scene);
    }

    void Node::RemoveChild(const rtm_ptr<NodeBase> &child, bool internal) {
        if (internal)
            this->internal_hierarchy.erase(
                std::remove(this->internal_hierarchy.begin(),
                            this->internal_hierarchy.end(), child),
                this->internal_hierarchy.end());
        else
            this->hierarchy.erase(std::remove(this->hierarchy.begin(), this->hierarchy.end(), child),
                                  this->hierarchy.end());
    }

    SHObject_Base_Impl(Actor)

    SHObject_Base_Impl(Entity)

    SHObject_Base_Impl(Scene)

    SHObject_Base_Impl(RootNode)

    SHObject_Base_Impl(World)

    World::World() : systemManager(manager) {
        this->root = manager.ConstructNode<RootNode>();
        this->root->SetWorld(this);
    }

    void World::Destroy(NodeBase *node) {
        // destroy all children
        if (Node *n = dynamic_cast<Node *>(node)) {
            for (auto &child : n->GetHierarchy()) {
                Destroy(child.Get());
            }
        }

        node->GetParent()->RemoveChild(node);

        // destroy the node
        manager.DestroyNode(node);

    }

}