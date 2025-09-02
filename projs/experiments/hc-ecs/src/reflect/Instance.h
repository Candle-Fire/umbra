
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once
#include "Type.h"

namespace sonic {
    namespace reflect {
        /**
         * A container for an instance of a class created by its' type reflection.
         * Move only.
         */
        class Instance {
        public:

            Instance() = default;
            Instance(Reflect* inst); // Transfer ownership
            Instance(Instance const& r);
            Instance(Instance&& r);
            virtual ~Instance();

            Instance& Clone(Instance const& r);
            Instance& Clone(Reflect const* r);

            Instance& operator=(Instance const& r) { return Clone(r); }
            Instance& operator=(Instance&& r);

            inline bool operator==(Reflect const* rhs) const { return Inst == rhs;}
            inline bool operator!=(Reflect const* rhs) const { return Inst != rhs;}

            inline bool IsPresent() const { return Inst != nullptr; }

            bool IsEquivalent(Instance const& r) const; // Check reflected data

            Reference GetInstanceReference() const;

            inline Meta const* GetInstanceMeta() const;

            virtual Meta const* GetAllowedMeta() const;

            void Set(Reflect* target);

            void Create(Meta const* meta) {
                delete Inst;
                Inst = meta->CreateInstance();
            }

            template<typename T, typename ... ConstructorParams>
            void Create(ConstructorParams&&... params) {
                delete Inst;
                Inst = new T(std::forward<ConstructorParams>(params)...);
            }

            void Destroy();

            inline Reflect* Get() { return Inst; }

            inline Reflect const* Get() const { return Inst; }

            template<typename T>
            inline T* GetAs() { return Cast<T>(Inst); }

            template<typename T>
            inline T const* GetAs() const { return Cast<T>(Inst); }

            template<typename T>
            inline T* TryGetAs() const { return TryCast<T>(Inst); }

            template<typename T>
            inline T const* TryGetAs() const { return TryCast<T>(Inst); }

            Reflect* Inst = nullptr;

        };


        /**
         * A default implementation of an Instance, for fastest possible instantiation.
         * @tparam T the type of the class that the instance holds.
         */
        template<typename T>
        class Instance_t final : public Instance {
        public:

            using Instance::Instance;

            explicit Instance_t(T* inst) : Instance(inst) {}

            explicit Instance_t(Instance const& r) {
                Inst = r.GetInstanceMeta()->CreateInstance();
                Inst->GetMeta()->CopyProperties(Inst, r.Inst);
            }

            explicit Instance_t(Instance_t const& r) {
                Inst = r.GetInstanceMeta()->CreateInstance();
                Inst->GetMeta()->CopyProperties(Inst, r.Inst);
            }

            explicit Instance_t(Instance&& r) : Instance(r.Inst) {
                r.Inst = nullptr;
            }

            Instance_t& operator=(Instance const& r) { return Clone(r); }
            Instance_t& operator=(Instance_t const& r) { Clone(r.Inst); return *this; }
            Instance_t& operator=(Instance&& r) { delete Inst; Inst = r.Inst; r.Inst = nullptr; return *this; }
            Instance_t& operator=(Instance_t&& r) { delete Inst; Inst = r.Inst; r.Inst = nullptr; return *this; }

            virtual Meta const* GetAllowedMeta() const override { return T::GetStaticReference(); }

            inline T* Get() { return static_cast<T*>(Inst); }
            inline T const* Get() const { return static_cast<T const*>(Inst); }
            inline T* operator->() { return (T*) Inst; }
            inline T const* operator->() const { return (T const*) Inst; }

            template<typename X>
            bool operator==(X const* r) const { return Inst == r; }

            template<typename X>
            bool operator!=(X const* r) const { return Inst != r; }

            template<typename X>
            inline bool operator==(Instance_t<X> const& r) const { return Inst == r.Inst; }

            template<typename X>
            inline bool operator!=(Instance_t<X> const& r) const { return Inst != r.Inst; }

            void Create() { Instance::Create<T>(); }

            template<typename X, typename... ConstructorParams>
            void Create(ConstructorParams&&... params) {
                Instance::Create<X>(std::forward<ConstructorParams>(params)...);
            }

            void Create(Meta const* meta) {
                Instance::Create<T>(meta);
            }

        private:

            Instance_t(Reflect* inst) = delete;


        };
    }
}
