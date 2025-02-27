#include "shadow/entitiy/graph/managers.h"

std::size_t std::hash<SH::Entities::Types>::operator()(const SH::Entities::Types &s) const noexcept {
    auto hash = SH::StableHash(s.data(), s.size());
    return hash.getHash();
}

namespace SH::Entities {

  template<SH::Entities::NodeBaseType Component>
  void *NodeManager::get_component(const EntityId &entity) {
      return get_component(entity, Node::TypeId());
  }

  void NodeManager::add_component(const EntityId &entity, const NodeBase &child) {
      Types types = std::vector(entity_index.at(entity).archetype->types);
      types.push_back(child.GetTypeId());

      auto archetype = get_archetype(types);

  }

  const NodeManager::Archetype &NodeManager::get_archetype(const Types types) {
      if(archetype_index.contains(types)){
          return *archetype_index.at(types);
      }
      else{
          auto &a = archetype.emplace_back(Archetype(next_archetype_id++));
          archetype_index.emplace(types, &a);
          for (auto &type: types) {

              if(!component_index.contains(type)){
                  component_index.insert({type, ArchetypeMap{}}).first->second;
              }

              auto &archetypeList = component_index.at(type);
              archetypeList.insert({a.id,{0}});
          }
      }
  }

  void* NodeManager::get_component(const EntityId &entity, const ComponentTypeId &component) {
      Record &record = entity_index[entity];
      Archetype *archetype = record.archetype;
      ArchetypeMap archetypes = component_index[component];
      if (archetypes.count(archetype->id) == 0) {
          return nullptr;
      }
      ArchetypeRecord &a_record = archetypes[archetype->id];
      return archetype->children[a_record.column][record.row];
  }

  SHObject_Base_Impl(RootNode)

  SHObject_Base_Impl(World)

  World::World() : systemManager(manager) {
      //this->root = manager.ConstructNode<RootNode>();
      //this->root->SetWorld(this);
  }

  void World::Destroy(NodeBase *node) {
      // destroy all children
      if (Node * n = dynamic_cast<Node *>(node)) {
          for (auto &child : n->GetHierarchy()) {
              Destroy(child.Get());
          }
      }

      node->GetParent()->RemoveChild(node);

      // destroy the node
      //manager.DestroyNode(node);

  }

}