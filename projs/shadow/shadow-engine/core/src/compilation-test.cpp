//
// Created by dpeter99 on 23/08/2023.
//

#include "dxmath.h"

#ifdef DX12
#include <d3d12.h>


void testFunc() {
    DirectX::XMMATRIX const test{};

    IID iid{};
    ID3D12Device *dev;
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, iid, (void **) &dev);
}
#endif