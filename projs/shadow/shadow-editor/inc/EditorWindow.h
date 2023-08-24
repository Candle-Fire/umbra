//
// Created by dpete on 07/12/2022.
//
#pragma once

#include "SHObject.h"

namespace ShadowEngine::Editor {

    class EditorWindow : public ShadowEngine::SHObject{
    SHObject_Base(EditorWindow)
    public:
        virtual void Draw()= 0;
    };

}
