#include "SFFElement.h"


 namespace Shadow::SFF {

        SFFElement* SFFElement::GetFirstChild()
        {
            return children.size() > 0 ? children.begin()->second : nullptr;
        }

        SFFElement* SFFElement::GetChildByIndex(int index)
        {
            SFFElement::ChildrenMap::iterator it = children.begin();
            if (it == children.end())
                return nullptr;

            for (size_t i = 0; i < index; i++)
            {
                it++;
            }
            return it->second;
        }

        SFFElement* SFFElement::GetChildByName(std::string name)
        {
            SFFElement::ChildrenMap::iterator it = children.find(name);
            if (it != children.end()) {
                return it->second;
            }
            return nullptr;
        }

		SFFElement::~SFFElement(){}

}