#include "SFFParser.h"
#include "../../shadow-utility/src/string-helpers.h"

#include <fstream>

namespace Shadow::SFF {

	SFFElement* SFFParser::ReadFromStream(std::istream& stream)
	{
		auto version = ReadVersionFromHeader(stream);
		if (version.invalid) {
			//SH_CORE_WARN("Shadow File is invalid");
			return nullptr;
		}


		//The current node that we are building
		auto* context = new SFFElement;
		context->name = "root";

		//Top level Element
		SFFElement* base = context;

		//The new node that will be a child of the context
		SFFElement* current = nullptr;


		std::string buffer;

		char c;
		while (!stream.eof())
		{
			stream.get(c);

			switch (c) {
			case ':':
				//The stuff in the buffer is a parameter name
				//std::cout << "Name: " << buffer;
				current = new SFFElement;
				current->name = buffer;
				current->parent = context;

				buffer = "";
				break;

			case '{':
				//Start of a new block
				current->isBlock = true;
				context = current;

				current = nullptr;
				break;

			case ',':
				// End of a property
				if (!current->isBlock) {
					//The stuff is the value
					current->value = buffer;
					current->parent = context;
					current->isBlock = false;
				}
				buffer = "";

				context->children[current->name] = current;

				current = nullptr;
				break;

			case '}':
				if (current != nullptr) {
					// End of a block
					current->parent = context;
					context->children[current->name] = current;
				}
				current = context;
				context = current->parent;

				break;

			default:
				if (std::isspace(c) == 0)
				{
					buffer += c;
				}
				break;
			}

		}

		//std::cout << "END" << std::endl;

		return base;
	}

	SFFVersion SFFParser::ReadVersionFromHeader(std::istream& stream) {
		std::string line;
		std::getline(stream, line);
		auto parts = explode(line, '_');
		if (parts[0] != "ShadowFileFormat") {
			return SFFVersion(-1, -1, -1);
		}
		else {
			int mayor = std::stoi(parts[1]);
			int minor = std::stoi(parts[2]);
			int patch = std::stoi(parts[3]);
			return SFFVersion(mayor, minor, patch);
		}

	}



	SFFElement* SFFParser::ReadFromFile(std::string path)
	{
		std::ifstream inputFileStream(path);

		if (errno)
		{
			//SH_CORE_ERROR("Error: {0} | File: {1}", strerror(errno), path);
			//std::cerr << "Error: " << strerror(errno) << std::endl;
			//std::cerr << "File: " << path << std::endl;
			return nullptr;
		}

		return ReadFromStream(inputFileStream);
	}
}

