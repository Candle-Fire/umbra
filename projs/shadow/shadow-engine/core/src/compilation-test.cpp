//
// Created by dpeter99 on 23/08/2023.
//

#include <algorithm>
#include <utility>

#include <DirectXMath.h>

#define XSTR(x) STR(x)
#define STR(x) #x

#pragma message "The value of ABC: " XSTR(DIRECTX_MATH_VERSION)

void testFunc() {
    DirectX::XMMATRIX const test{};
}