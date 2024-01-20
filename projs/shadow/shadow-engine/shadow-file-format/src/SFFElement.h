#pragma once

#include <string>
#include <map>
#include <list>


 namespace Shadow::SFF {

    class SFFElement
	{
	public:
		SFFElement* parent;

		std::string name;

		bool isBlock;

		std::string value;
		typedef std::map<std::string, SFFElement*> ChildrenMap;

        ChildrenMap children;

		std::string GetStringProperty(std::string name);

        SFFElement* GetFirstChild();

        SFFElement* GetChildByIndex(int index);

        SFFElement* GetChildByName(std::string name);

		~SFFElement();

	};

}