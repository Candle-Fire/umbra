#pragma once

#include <DirectXMath.h>
#include <limits>

#define limit(x, y) std::numeric_limits<x>::y()
namespace rx {
    struct AABB {
        enum Intersect {
            OUTSIDE,
            INTERSECTS,
            INSIDE,
        };

        DirectX::XMFLOAT3 min;
        DirectX::XMFLOAT3 max;
        uint32_t layerMask = ~0;
        uint32_t userData = 0;

        AABB( const DirectX::XMFLOAT3& min = DirectX::XMFLOAT3(limit(float, max), limit(float, max), limit(float, max)), const DirectX::XMFLOAT3& max = DirectX::XMFLOAT3(limit(float, lowest), limit(float, lowest), limit(float, lowest)))
            : min(min), max(max) {}

        void CreateFromHalfWidth(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& halfWidth);
        AABB Transform(const glm::mat4& mat) const;
        AABB Transform(const DirectX::XMFLOAT4X4& mat) const;
        DirectX::XMFLOAT3 GetCenter() const;
        DirectX::XMFLOAT3 GetHalfWidth() const;
        glm::mat4 GetAsBoxMatrix() const;
        float GetArea() const;
        float GetRadius() const;

        Intersect Intersects2D(const AABB& other) const;
        Intersect Intersects(const AABB& other) const;
        bool Intersects(const DirectX::XMFLOAT3& point) const;

        AABB operator* (float a);

        static AABB Merge(const AABB& a, const AABB& b);

        constexpr DirectX::XMFLOAT3 GetMin() const { return min; }
        constexpr DirectX::XMFLOAT3 GetMax() const { return max; }
        constexpr DirectX::XMFLOAT3 Corner(uint32_t idx) const {
            switch (idx) {
                case 0: return min;
                case 1: return DirectX::XMFLOAT3(min.x, max.y, min.z);
                case 2: return DirectX::XMFLOAT3(min.x, max.y, max.z);
                case 3: return DirectX::XMFLOAT3(min.x, min.y, max.z);
                case 4: return DirectX::XMFLOAT3(max.x, min.y, min.z);
                case 5: return DirectX::XMFLOAT3(max.x, max.y, min.z);
                case 6: return max;
                case 7: return DirectX::XMFLOAT3(max.x, min.y, max.z);
            }

            return DirectX::XMFLOAT3(0, 0, 0);
        }

        constexpr bool IsValid() const {
            if (min.x > max.x || min.y > max.y || min.z > max.z) return false;
            return true;
        }
    };

    struct Frustum {
        DirectX::XMFLOAT4 planes[6];

        void Create(const glm::mat4& mat);
        bool CheckPoint(const DirectX::XMFLOAT3&) const;
        bool CheckSphere(const DirectX::XMFLOAT3&, float) const;

        enum Intersect {
            OUTSIDE,
            INTERSECTS,
            INSIDE
        };

        Intersect CheckBox(const AABB& box) const;
        bool CheckBoxFast(const AABB& box) const;

        const DirectX::XMFLOAT4& GetNearPlane() const;
        const DirectX::XMFLOAT4& GetFarPlane() const;
        const DirectX::XMFLOAT4& GetLeftPlane() const;
        const DirectX::XMFLOAT4& GetRightPlane() const;
        const DirectX::XMFLOAT4& GetToplane() const;
        const DirectX::XMFLOAT4& GetBottomPlane() const;
    };
}