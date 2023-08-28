#include "../inc/shadow/SHObject.h"

uint64_t SH::SHObject::GenerateId() noexcept {
    static uint64_t count = 0;
    return ++count;
}
