#pragma once

/**
 * A temporary header that contains some of the core transform logic.
 *
 */

#include <xmmintrin.h>

namespace Math {

    struct alignas(16) Vector {
        Vector() = default;

        union {
            struct { float x, y, z, w; };
            __m128 data;
        };
    };

    struct alignas(16) Quaternion {

        inline Quaternion() = default;

        union {
            struct { float x, y, z, w; };
            __m128 data;
        };
    };

    class Transform {
    public:
        Transform() = default;

        const Vector& GetTranslation() const { return translation; }
        const Quaternion& GetRotation() const { return rotation; }

        inline Vector GetRightVector() const;
        inline Vector GetForwardVector() const;
        inline Vector GetUpVector() const;

    private:
        Quaternion rotation;
        Vector translation;
        Vector scale;
    };
}