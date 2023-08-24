#include "../inc/SHObject.h"

uint64_t ShadowEngine::SHObject::GenerateId() noexcept {
    static uint64_t count = 0;
    return ++count;
}
