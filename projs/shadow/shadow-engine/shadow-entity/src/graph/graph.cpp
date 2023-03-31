#include "graph/graph.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(NodeBase)

    void NodeBase::SetParent(const rtm_ptr<NodeBase> &parent) {
        this->parent = parent;
    }

    void NodeBase::SetScene(const rtm_ptr<Scene> &scene) {
        this->m_scene = scene;
    }

    void NodeBase::SetWorld(World *world) {
        this->m_world = world;
    }

    SHObject_Base_Impl(LeafNode)

    SHObject_Base_Impl(Component)

    SHObject_Base_Impl(Node)

    void Node::AddChild(const rtm_ptr<NodeBase> &child) {
        this->hierarchy.push_back(child);
        // set the child's parent to this
        child->SetParent(this);
        // set the child's scene to this
        child->SetScene(this->m_scene);
    }

    SHObject_Base_Impl(Actor)

    SHObject_Base_Impl(Scene)

    SHObject_Base_Impl(World)

    void World::AddScene(rtm_ptr<Scene> scene) {
        scenes.push_back(scene);
        // set the scene's world to this
        scene->SetWorld(this);
    }

}