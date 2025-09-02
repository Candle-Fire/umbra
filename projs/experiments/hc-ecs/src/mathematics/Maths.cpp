#include <mathematics/Maths.h>

namespace sonic::maths {

    XMVECTOR const Transformation::Xv0001 = XMVectorSet(0,0,0,1);
    Transformation const Transformation::Identity = Transformation(XMQuaternionIdentity(), XMFLOAT3(0,0,0), 1.0f);
}
