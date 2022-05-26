module;

import <string>;

import shadow_utils;

module Shadow.FileFormat:SFFElement;


namespace Shadow::SFF {


    SFFElement* SFFElement::GetFirstChild()
    {
        return children.size() > 0 ? children.begin()->second : nullptr;
    }

    SFFElement* SFFElement::GetChildByIndex(int index)
    {
        ChildrenMap::iterator it = children.begin();
        for (size_t i = 0; i < index; i++)
        {
            it++;
        }
        return it->second;
    }

    SFFElement* SFFElement::GetChildByName(std::string name)
    {
        ChildrenMap::iterator it = children.find(name);
        if (it != children.end()) {
            return it->second;
        }
        return nullptr;
    }

}