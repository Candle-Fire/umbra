#pragma once

#include <string>
#include <map>
#include <memory>

namespace shadowutil {

    // An object wrapper that behaves like smart pointers.
    // References to this object are counted, and the object will be destructed when there are no references.
    // This allows for automatic, safe and leak-free handling of all kinds of resources.
    // The AutoRelease behaviour can be adjusted.
    template <typename ObjectType>
    class RefCounter {
    public:

        // Preserves ObjectType instances in the current scope.
        // Use like lock_guard; RAII allows precise scoping rules.
        class AutoRelease {
        public:
            explicit AutoRelease() { RefCounter<ObjectType>::registerAutoRelease(); }

            AutoRelease(const AutoRelease&) = delete;
            AutoRelease& operator=(const AutoRelease&) = delete;

            ~AutoRelease() { RefCounter<ObjectType>::unregisterAutoRelease(); }

            // AutoRelease can only exist on the stack.
            void* operator new(size_t) = delete;
            void* operator new[](size_t) = delete;
        };

        // Use to access a ref counted object.
        // If exists, the object will be passed and reference counter increased.
        // Otherwise, args will be used to create.
        template<typename... Args>
        static RefCounter get(const std::string& identifier, Args&&... args) {
            auto iter = getMap().find(identifier);
            if (iter == getMap().end()) {
                const auto inserted = getMap().insert(
                    {
                        identifier,
                        typename ObjectPool::CountedObject {
                            std::make_unique<ObjectType>(std::forward<Args>(args)...), 0
                        }
                    }
                );
                iter = inserted.first;
            }

            auto& object = iter->second;
            ++object.references;
            return RefCounter { identifier, object.obj.get() };
        }

        RefCounter(RefCounter&& other) noexcept {
            identifier = std::move(other.identifier);
            objectPtr = other.objectPtr;
            other.identifier.clear();
        }

        RefCounter& operator=(RefCounter&& other) noexcept {
            std::swap(identifier, other.identifier);
            std::swap(objectPtr, other.objectPtr);
            return *this;
        }

        // When we reach 0 references and there's no auto release system set up, destroy the object.
        ~RefCounter() {
            if (identifier.empty()) return;

            const auto iter = getMap().find(identifier);
            if (--iter->second.references == 0 && objectPool.activePools == 0)
                getMap().erase(iter);
        }

        // Smart pointer emulation overloads.
        const ObjectType* operator->() const { return objectPtr; }
        const ObjectType& operator*() const { return *objectPtr; }

        static bool hasAutoRelease() { return objectPool.activePools != 0; }

    private:
        // The object pool that handles managing and counting objects.
        struct ObjectPool {
            struct CountedObject {
                std::unique_ptr<ObjectType> obj;
                size_t references;
            };

            using RefCountMap = std::map<std::string, CountedObject>;

            RefCountMap refCountMap;
            size_t activePools = 0;
        };

        RefCounter(std::string identifier, const ObjectType* obj) : identifier(std::move(identifier)), objectPtr(obj) {}

        static void unregisterAutoRelease() {
            if (--objectPool.activePools == 0) {
                using CountedObject = typename ObjectPool::CountedObject;
                static const auto removeZeroRef = [](const std::pair<const std::string, CountedObject> &pair) {
                    return pair.second.references == 0;
                };
                std::erase_if(getMap(), removeZeroRef);
            }
        }

        static void registerAutoRelease() { ++objectPool.activePools; }

        static typename ObjectPool::RefCountMap& getMap() { return objectPool.refCountMap; }

        static ObjectPool objectPool;

        std::string identifier;
        const ObjectType* objectPtr;
    };

    template <typename ObjectType>
    typename RefCounter<ObjectType>::ObjectPool RefCounter<ObjectType>::objectPool {};
}