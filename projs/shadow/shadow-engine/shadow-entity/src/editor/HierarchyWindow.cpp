#include "editor/HierarchyWindow.h"

namespace ShadowEngine::Entities::Editor {

    EntitySystem* HierarchyWindow::entitySystem = nullptr;

    ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::Entity> selected_ent;
    ShadowEngine::Entities::rtm_ptr<ShadowEngine::Entities::Entity> selected_inspector;


    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;

    ImGuiTreeNodeFlags treeSelectableFlags(rtm_ptr<Entity> a, rtm_ptr<Entity> b) {
        ImGuiTreeNodeFlags flags = node_flags;
        if (a == b)
            flags |= ImGuiTreeNodeFlags_Selected;

        return flags;
    }

    void DrawTree(rtm_ptr<Entity> entity){
        ImGuiTreeNodeFlags flags = treeSelectableFlags(selected_ent, entity);
        if(entity->hierarchy.size() <= 0)
            flags |= ImGuiTreeNodeFlags_Leaf;


        if (ImGui::TreeNodeEx((void *) &entity, flags, entity->name.c_str())) {

            if (ImGui::IsItemClicked()) {
                selected_ent = entity;
                selected_inspector = entity;
            }

            for (auto &child: entity->hierarchy) {
                DrawTree(child);
            }

            ImGui::TreePop();
        }
    }

    void HierarchyWindow::DebugHierarchy() {
        static bool shown;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12);

        ImGui::Begin("Hierarchy", &shown, ImGuiWindowFlags_MenuBar);


        auto scene = entitySystem->GetActiveScene();
        DrawTree((rtm_ptr<Entity>)scene);


        ImGui::End();

        ImGui::PopStyleVar(2);
    }

    void HierarchyWindow::Inspector() {
        static bool shown = true;

        ImGui::Begin("Inspector", &shown, ImGuiWindowFlags_None);

        if (selected_ent) {

            std::string title = selected_ent->name;
            title += " (" + selected_ent->Type() + ")";
            ImGui::Text(title.c_str(), "");

            ImGui::Separator();

            bool top_open = ImGui::TreeNodeEx(selected_ent->name.c_str(), treeSelectableFlags(selected_ent, selected_inspector));
            if (ImGui::IsItemClicked())
                selected_inspector = selected_ent;
            if (top_open)
            {
                for (auto & entity : selected_ent->internalHierarchy)
                {
                    bool open = ImGui::TreeNodeEx(entity->name.c_str(), treeSelectableFlags(entity, selected_inspector));

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
        }

        ImGui::End();
    }


}