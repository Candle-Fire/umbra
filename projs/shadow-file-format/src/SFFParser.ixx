module;

import <string>;
import <iostream>;

export module Shadow.FileFormat:SFFParser;

import :SFFElement;
import :SFFVersion;

export namespace Shadow::SFF {

	class SFFParser
	{
	public:

		static SFFElement* ReadFromStream(std::istream& stream);

		static SFFVersion ReadVersionFromHeader(std::istream& stream);

		static SFFElement* ReadFromFile(std::string path);
	};

}
