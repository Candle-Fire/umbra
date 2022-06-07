module;

#include<fstream>
#include <iomanip>

export module Shadow.FileFormat:SFFWriter;

import <string>;
//import <ostream>;
import <string>;
//import <iomanip>;

import shadow_utils;

import :SFFElement;
import :SFFVersion;


export namespace Shadow::SFF {

	class SFFWriter
	{
	public:
		static void WriteFile(SFFElement& root, std::string path) {

            try
            {
                std::ofstream writer;
                writer.open(path);
                writer << "ShadowFileFormat_1_0_0";

                int depth = 0;
                WriteElement(writer, root, 0);

                writer.close();
            }
            catch (std::exception e) {

            }
		}
      

        static void WriteElement(std::ofstream& w, SFFElement& e, int depth) {

            std::string head = (e.name + (e.isBlock ? ":{" : ":"));
            //head = head.PadLeft(depth + head.Length, '\t');
            w << std::setw(depth*4) << head;
            
            if (e.isBlock)
            {
                w << "\n";
                for(auto property : e.children)
                {
                    WriteElement(w, *property.second, depth++);
                }
                w << std::setw(depth*4) << "}\n";
            }
            else
            {
                w << (e.value + ",") << "\n";
            }
            
        }

	};


}