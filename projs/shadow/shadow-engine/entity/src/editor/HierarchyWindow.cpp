#include "shadow/entity/editor/HierarchyWindow.h"
#include "shadow/entity/EntitySystem.h"
#include "shadow/entity/graph/graph.h"

namespace SH::Entities::Editor {

    std::weak_ptr<SH::Entities::EntitySystem> HierarchyWindow::entitySystem;

    SH::Entities::rtm_ptr<SH::Entities::NodeBase> selected_ent;
    SH::Entities::rtm_ptr<SH::Entities::NodeBase> selected_inspector;

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

        const char *name = (const char *) node->GetType().c_str();

        Node *h_node = dynamic_cast<Node *>(node.Get());

        if (h_node == nullptr || h_node->GetHierarchy().empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        auto *h_actor = dynamic_cast<Actor *>(node.Get());
        if (h_actor != nullptr) {
            name = h_actor->GetName().c_str();
        }

        if (ImGui::TreeNodeEx(node.GetInternalPointer(), flags, "%s", name)) {

            if (ImGui::IsItemClicked()) {
                selected_ent = node;
                selected_inspector = node;
            }

            if (h_node != nullptr) {
                for (auto &child : h_node->GetHierarchy()) {
                    DrawTree(child);
                }
            }

            ImGui::TreePop();
        }
    }

    void HierarchyWindow::DebugHierarchy() {
        static bool shown;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12);

        ImGui::Begin("Hierarchy", &shown, ImGuiWindowFlags_MenuBar);

        if (!entitySystem.expired()) {
            auto a = entitySystem.lock();
            auto root = a->GetWorld().GetRoot();

            //Draw each scene in the world
            for (auto &scene : root->GetHierarchy()) {
                DrawTree(scene);
            }
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
