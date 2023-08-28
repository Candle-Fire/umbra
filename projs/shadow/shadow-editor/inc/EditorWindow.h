//
// Created by dpete on 07/12/2022.
//
#pragma once

#include "shadow/SHObject.h"

namespace SH::Editor {

    class EditorWindow : public SH::SHObject{
    SHObject_Base(EditorWindow)
    public:
        virtual void Draw()= 0;
    };

}
