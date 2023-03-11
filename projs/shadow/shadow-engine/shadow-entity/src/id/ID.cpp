#include <id/ID.h>
#include <atomic>

namespace SE {
    static std::atomic<uint64_t> entityID = 1;
    EntityID EntityID::Generate() {
        EntityID id(entityID++);
        return id;
    }

    static std::atomic<uint64_t> componentID = 1;
    ComponentID ComponentID::Generate() {
        ComponentID id(componentID++);
        return id;
    }
}