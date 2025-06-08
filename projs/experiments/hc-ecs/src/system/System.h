
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

namespace sonic {

    /**
     * Component-managing system. A given System manages (stores, updates) all Components of a given type.
     *
     */
    struct System {
        virtual ~System();
        virtual void init() {}
        virtual void serialize(struct OutputMemoryStream& serializer) = 0;
        virtual void deserialize(struct InputMemoryStream& serialize, const struct EntityMap& map, uint32_t version) = 0;
        virtual void preReload(OutputMemoryStream& serializer) {}
        virtual void postReload(InputMemoryStream& serializer) {}
        virtual void update(float timeDelta) = 0;
        virtual void lateUpdate(float timeDelta) {}
        virtual struct World& getWorld() = 0;
        virtual void startGame() {}
        virtual void stopGame() {}
        virtual uint32_t getVersion() const { return -1; };
    };
}

