
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once

#include <stddef.h>
#include <string_view>
#include <vector>

template <typename Str>
constexpr size_t hashString(const Str& toHash)
{
    // For this example, I'm requiring size_t to be 64-bit, but you could
    // easily change the offset and prime used to the appropriate ones
    // based on sizeof(size_t).
    static_assert(sizeof(size_t) == 8);
    // FNV-1a 64 bit algorithm
    size_t result = 0xcbf29ce484222325; // FNV offset basis

    for (char c : toHash) {
        result ^= c;
        result *= 1099511628211; // FNV prime
    }

    return result;
}

template <size_t N>
constexpr size_t hashString(char const (&toHash)[N])
{
    return hashString(std::string_view(toHash));
}

#define SONIC_MODULE(name)                                                      \
constexpr static size_t const ModuleID = hashString(#name);                     \
virtual size_t GetModuleID() const override final { return name::ModuleID; }

namespace sonic {
    /**
     * A way to reference singleton plugin modules using the Reflection system.
     */
    class Module {
    public:

        Module() = default;
        Module(Module const&) = default;
        virtual ~Module() = default;

        Module& operator=(Module const&) = default;

        virtual size_t GetModuleID() const = 0;
    };

    /**
     * Keeps track of all active Modules in use.
     ! The SONIC_MODULE macro must be used for the GetModule generic function to work.
     */
    class ModuleRegistry {
    public:

        ModuleRegistry() {}
        ~ModuleRegistry();

        void RegisterModule(Module* mod);
        void UnregisterModule(Module* mod);

        template<typename T>
        inline T* GetModule() const {
            for (auto mod : RegisteredModules)
                if (mod->GetModuleID() == T::ModuleID)
                    return reinterpret_cast<T*>(mod);

            return nullptr;
        }

        std::vector<Module*>   RegisteredModules;
    };
}
