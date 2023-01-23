#include "editor/HierarchyWindow.h"

namespace ShadowEngine::Entities::Editor {

    EntitySystem* HierarchyWindow::entitySystem = nullptr;

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;

    void DrawTree(rtm_ptr<Entity> entity){
        ImGuiTreeNodeFlags flags = node_flags;
        if(entity->hierarchy.size() <= 0)
            flags |= ImGuiTreeNodeFlags_Leaf;


        if (ImGui::TreeNodeEx((void *) &entity, flags, entity->name.c_str())) {
            for (auto &child: entity->hierarchy) {

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
                //if (selected_ent == entity)
                //    node_flags |= ImGuiTreeNodeFlags_Selected;

                DrawTree(child);

                if (ImGui::IsItemClicked()) {
                    //selected_ent = entity;
                    //selected_inspector = entity;
                }

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

}