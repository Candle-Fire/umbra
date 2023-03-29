#include "graph/graph.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(NodeBase)

    SHObject_Base_Impl(LeafNode)

    SHObject_Base_Impl(Component)

    SHObject_Base_Impl(Node)

    void Node::AddChild(rtm_ptr<NodeBase> child) {
        hierarchy.push_back(child);
        // set the child's parent to this
        child->SetParent(this);
    }

    void Node::AddInternalChild(rtm_ptr<NodeBase> child) {
        internal_hierarchy.push_back(child);
        // set the child's parent to this
        child->SetParent(this);

    }

    SHObject_Base_Impl(Actor)

    SHObject_Base_Impl(Scene)

    SHObject_Base_Impl(RootNode)

    void RootNode::AddScene(rtm_ptr<Scene> scene) {
        scenes.push_back(scene);
        // set the scene's parent to this
        scene->SetParent(this);
    }

}