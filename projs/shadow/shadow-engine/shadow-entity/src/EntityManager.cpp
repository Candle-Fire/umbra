
#include "EntityManager.h"
#include "Entity.h"

namespace ShadowEngine::Entities {

	EntityManager* EntityManager::Instance = nullptr;

    void EntityManager::RemoveEntity(const int entityIndex, const int typeID) {
        //Lookup of the entity to be removed
        Entity* entity = this->m_EntityLUT[entityIndex];
        //Invalidate the UID
        entity->m_runtimeUID = Entity::INVALID_UID;
        // unmap entity id
        ReleaseIndex(entityIndex);

        assert(entity != nullptr && "FATAL: Trying to remove a entity that doesn't exsist");

        entity->~Entity();

        // release object memory
        GetComponentContainer(typeID)->DestroyObject((void*)entity);
    }

    IEntityContainer *EntityManager::GetComponentContainer(int typeID) {
        auto it = this->m_EntityContainerRegistry.find(typeID);
        IEntityContainer* cc = nullptr;

        if (!(it == this->m_EntityContainerRegistry.end()))
            cc = static_cast<IEntityContainer*>(it->second);

        assert(cc != nullptr && "Failed to get ComponentContainer<Type>!");
        return cc;
    }

    int EntityManager::AssignIndexToEntity(Entity *component) {
        int i = 0;
        if (LUTFragm) {
            i = LUTFragmFree.back();
            LUTFragmFree.pop_back();
            if (LUTFragmFree.empty()) {
                LUTFragm = false;
            }
        }
        else {
            i = LUTNextFree;
            LUTNextFree++;
            if (!(i < m_EntityLUT.size())) {
                this->m_EntityLUT.resize(this->m_EntityLUT.size() + ENTITY_LUT_GROW, nullptr);
            }
        }

        /*
        for (; i < this->m_EntityLUT.size(); ++i)
        {
            if (this->m_EntityLUT[i] == nullptr)
            {
                this->m_EntityLUT[i] = component;
                return i;
            }
        }
        */

        // increase component LUT size


        this->m_EntityLUT[i] = component;
        return i;
    }

    void EntityManager::ReleaseIndex(int id) {
        assert(id < this->m_EntityLUT.size() && "Invalid component id");

        //If this free is from the middle of the LUT
        //We record that the LUT is fragmented
        if (id != this->m_EntityLUT.size() - 1) {
            LUTFragm = true;
            LUTFragmFree.push_back(id);
        }

        this->m_EntityLUT[id] = nullptr;
    }

    EntityManager::EntityManager() {
        Instance = this;
    }

}