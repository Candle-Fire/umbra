#pragma once

#include <DirectXMath.h>
#include <limits>

#define limit(x, y) std::numeric_limits<x>::y()
namespace rx {

    struct Sphere;
    struct Ray;
    struct Plane;
    struct Capsule;
    struct AABB;
    struct Frustum;

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
        bool Intersects(const Ray& ray) const;
        bool Intersects(const Sphere& sphere) const;
        bool Intersects(const Frustum& frustum) const;

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

    struct Sphere {
        DirectX::XMFLOAT3 center;
        float radius;

        Sphere() : center(DirectX::XMFLOAT3(0, 0, 0)), radius(0) {}
        Sphere(const DirectX::XMFLOAT3& c, float r) : center(c), radius(r) {
            assert(radius >= 0);
        }

#define INTERSECT_WITH(x) \
        bool Intersects(const x& b) const; \
        bool Intersects(const x& b, float& dist) const; \
        bool Intersects(const x& b, float& dist, DirectX::XMFLOAT3& dir) const; \

        bool Intersects(const AABB& b) const;
        INTERSECT_WITH(Sphere);
        INTERSECT_WITH(Capsule);
        INTERSECT_WITH(Plane);
        INTERSECT_WITH(Ray);

        DirectX::XMFLOAT4X4 GetPlacementOrientation(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& norm) const;
    };

    struct Capsule {
        DirectX::XMFLOAT3 base = DirectX::XMFLOAT3(0, 0, 0);
        DirectX::XMFLOAT3 tip = DirectX::XMFLOAT3(0, 0, 0);
        float radius = 0;

        Capsule() = default;
        Capsule(const DirectX::XMFLOAT3& base, const DirectX::XMFLOAT3& tip, float radius) : base(base), tip(tip), radius(radius) {
            assert(radius >= 0);
        }
        Capsule(const Sphere& sphere, float height) : base(DirectX::XMFLOAT3(sphere.center.x, sphere.center.y - sphere.radius, sphere.center.z)), tip(DirectX::XMFLOAT3(base.x, base.y + height, base.z)), radius(sphere.radius) {
            assert(radius >= 0);
        }
        inline AABB GetAABB() const {
            DirectX::XMFLOAT3 halfWidth = DirectX::XMFLOAT3(radius, radius, radius);
            AABB baseAABB;
            baseAABB.CreateFromHalfWidth(base, halfWidth);
            AABB tipAABB;
            tipAABB.CreateFromHalfWidth(tip, halfWidth);
            AABB result = AABB::Merge(baseAABB, tipAABB);
            assert(result.IsValid());
            return result;
        }

        bool Intersects(const Capsule& b, DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& norm, float& depth) const;
        INTERSECT_WITH(Sphere);
        INTERSECT_WITH(Plane);
        INTERSECT_WITH(Ray);
        bool Intersects(const DirectX::XMFLOAT3& point);

        DirectX::XMFLOAT4X4 GetPlacementOrientation(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal) const;
    };

    struct Plane {
        DirectX::XMFLOAT3 origin = {};
        DirectX::XMFLOAT3 normal = {};
        DirectX::XMFLOAT4X4 projection = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

        bool Intersects(const Capsule& b, DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& norm, float& depth) const;
        INTERSECT_WITH(Sphere);
        INTERSECT_WITH(Capsule);
        INTERSECT_WITH(Ray);
    };

    struct Ray {
        DirectX::XMFLOAT3 origin;
        DirectX::XMFLOAT3 direction;
        DirectX::XMFLOAT3 reflect;
        float tMin = 0;
        float tMax = std::numeric_limits<float>::max();

        bool Intersects(const AABB& b) const;
        INTERSECT_WITH(Sphere);
        INTERSECT_WITH(Capsule);
        INTERSECT_WITH(Plane);

        void CreateFromPoints(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);

        DirectX::XMFLOAT4X4 GetPlacementOrientation(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& norm) const;
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