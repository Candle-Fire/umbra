#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "SFFElement.h"
#include "SFFVersion.h"

namespace Shadow::SFF {

	class SFFWriter
	{
	public:

        static void WriteFile(SFFElement& root, std::string path);

        static void WriteElement(std::ostream& w, SFFElement& e, int &depth);



	};

}


