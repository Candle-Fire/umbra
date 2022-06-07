module;

export module Shadow.FileFormat:SFFElement;

import <string>;
import <map>;
import <list>;

 namespace Shadow::SFF {

    export class SFFElement
	{
	public:
		SFFElement* parent;

		std::string name;

		bool isBlock;

		std::string value;
		typedef std::map<std::string, SFFElement*> ChildrenMap;

        ChildrenMap children;

		std::string GetStringProperty(std::string name);

        SFFElement* GetFirstChild()
        {
            return children.size() > 0 ? children.begin()->second : nullptr;
        }

        SFFElement* GetChildByIndex(int index)
        {
            ChildrenMap::iterator it = children.begin();
            if (it == children.end())
                return nullptr;

            for (size_t i = 0; i < index; i++)
            {
                it++;
            }
            return it->second;
        }

        SFFElement* GetChildByName(std::string name)
        {
            ChildrenMap::iterator it = children.find(name);
            if (it != children.end()) {
                return it->second;
            }
            return nullptr;
        }

        ~SFFElement() {
            for (auto var : children)
            {
                delete var.second;
            }
        }

	};

}