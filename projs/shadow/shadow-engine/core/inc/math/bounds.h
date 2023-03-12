#pragma once
#include <math/transform.h>

/**
 * A temporary header that contains some of the bounds implementations.
 */

namespace Math {

    /**
     * A bounding box that can be rotated freely.
     *  Can be used as the collision box for an entity.
     */
    struct OrientedBB {

        OrientedBB() = default;


        Quaternion orientation;
        Vector center;
        Vector extent;
    };
}