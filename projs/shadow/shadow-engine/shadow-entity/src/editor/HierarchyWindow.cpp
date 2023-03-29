#include "editor/HierarchyWindow.h"

namespace ShadowEngine::Entities::Editor {

    EntitySystem *HierarchyWindow::entitySystem = nullptr;

    ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::NodeBase> selected_ent;
    ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::NodeBase> selected_inspector;

    ImGuiTreeNodeFlags node_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;

    ImGuiTreeNodeFlags treeSelectableFlags(rtm_ptr<NodeBase> a, rtm_ptr<NodeBase> b) {
        ImGuiTreeNodeFlags flags = node_flags;
        if (a == b)
            flags |= ImGuiTreeNodeFlags_Selected;

        return flags;
    }

    void DrawTree(rtm_ptr<NodeBase> node) {
        ImGuiTreeNodeFlags flags = treeSelectableFlags(selected_ent, node);
        // If the node can't be casted to a Node, it's a leaf
        Node *h_node = dynamic_cast<Node *>(node.Get());
        if (h_node == nullptr) {
            ImGui::TreeNodeEx((void *) &node,
                              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | flags,
                              node->GetType().c_str());
            return;
        }

        if (h_node->GetHierarchy().size() <= 0)
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (ImGui::TreeNodeEx((void *) &node, flags, node->GetType().c_str())) {

            if (ImGui::IsItemClicked()) {
                selected_ent = node;
                selected_inspector = node;
            }

            for (auto &child : h_node->GetHierarchy()) {
                DrawTree(child);
            }

            ImGui::TreePop();
        }
    }

    void HierarchyWindow::DebugHierarchy() {
        static bool shown;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12);

        ImGui::Begin("Hierarchy", &shown, ImGuiWindowFlags_MenuBar);

        auto root = entitySystem->GetRoot();

        //Draw each scene in the root
        for (auto &scene : root->GetScenes()) {
            DrawTree(scene);
        }

        ImGui::End();

        ImGui::PopStyleVar(2);
    }

    void HierarchyWindow::Inspector() {
        static bool shown = true;

        ImGui::Begin("Inspector", &shown, ImGuiWindowFlags_None);

/*        if (selected_ent) {

            std::string title = selected_ent->name;
            title += " (" + selected_ent->Type() + ")";
            ImGui::Text(title.c_str(), "");

            ImGui::Separator();

            bool top_open =
                ImGui::TreeNodeEx(selected_ent->name.c_str(), treeSelectableFlags(selected_ent, selected_inspector));
            if (ImGui::IsItemClicked())
                selected_inspector = selected_ent;
            if (top_open) {
                for (auto &entity : selected_ent->internalHierarchy) {
                    bool
                        open = ImGui::TreeNodeEx(entity->name.c_str(), treeSelectableFlags(entity, selected_inspector));

                    if (ImGui::IsItemClicked())
                        selected_inspector = entity;

                    if (open) {
                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }

            ImGui::Separator();

            if (selected_inspector) {
                //InspectorSystem::DrawEntityInspector(selected_inspector);
            }
        }*/

        ImGui::End();
    }

}
