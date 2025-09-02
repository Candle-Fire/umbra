
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

namespace sonic {
    struct LoadContext {
        LoadContext () = default;

        inline bool IsValid() const {
            return true;
        }
    };

    struct InitializationContext {

        inline bool IsValid() const {
            return true;
        }
    };


    enum class UpdateStep {
        FrameStart,
        PrePhysics,
        Physics,
        PostPhysics,
        FrameEnd,
        Paused,
        Count
    };

    enum class UpdatePriority {
        Highest = 0x00,
        High = 0x40,
        Medium = 0x80,
        Low = 0xC0,
        Disabled = 0xFF,
        Default = Medium
    };

    struct UpdatePriorityPair {
        inline UpdatePriorityPair(UpdateStep step) : step(step) {}
        inline UpdatePriorityPair(UpdateStep step, uint8_t priority) : step(step), priority(priority) {}
        inline UpdatePriorityPair(UpdateStep step, UpdatePriority priority) : step(step), priority((uint8_t)priority) {}
        UpdateStep step;
        uint8_t priority = (uint8_t) UpdatePriority::Default;
    };

    struct UpdatePriorityList {
        UpdatePriorityList() { Reset(); }
        template<typename... args>
        UpdatePriorityList(args&&... arg) {
            Reset();
            ((*this << static_cast<args&&>(arg)), ...);
        }

        inline bool IsStepEnabled(UpdateStep step) const {
            return priorities[(uint8_t) step] != (uint8_t) UpdatePriority::Disabled;
        }

        inline uint8_t GetPriority(UpdateStep step) const {
            return priorities[(uint8_t) step];
        }

        inline UpdatePriorityList& SetPriority(UpdatePriorityPair&& priority) {
            priorities[(uint8_t) priority.step] = priority.priority;
            return *this;
        }

        inline UpdatePriorityList& operator<<(UpdatePriorityPair&& pair) {
            priorities[(uint8_t) pair.step] = pair.priority;
            return *this;
        }

        inline void Reset() { memset(priorities, (uint8_t) UpdatePriority::Disabled, sizeof(priorities)); }

        uint8_t priorities[(size_t) UpdateStep::Count];
    };
}