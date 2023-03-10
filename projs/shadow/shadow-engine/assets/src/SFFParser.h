#pragma once

#include <string>
#include <iostream>

#include "SFFElement.h"
#include "SFFVersion.h"

namespace Shadow::SFF {

	class SFFParser
	{
	public:

		static SFFElement* ReadFromStream(std::istream& stream);

		static SFFVersion ReadVersionFromHeader(std::istream& stream);

		static SFFElement* ReadFromFile(std::string path);
	};

}
