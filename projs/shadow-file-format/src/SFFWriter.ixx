module;

//#include <iostream>;
//#include <fstream>;

export module Shadow.FileFormat:SFFWriter;

import <string>;
import <iostream>;
import <fstream>;
import shadow_utils;

import :SFFElement;
import :SFFVersion;

namespace Shadow::SFF {

	export class SFFParser
	{
	public:

        static void WriteFile(SFFElement& root, std::string path)
        {
            std::ofstream writer(path);
            writer << "ShadowFileFormat_1_0_0" << std::endl;

            int depth = 0;
            WriteElement(writer, root, depth);

            writer.flush();
        }

        static void WriteElement(std::ostream& w, SFFElement& e, int &depth)
        {
            std::string head = (e.name + (e.isBlock ? ":{" : ":"));
            //head = head.PadLeft(depth + head.Length, '\t');
            head.insert(head.begin(), depth, '\t');
            w << head << std::endl;

            if (e.isBlock)
            {
                depth += 1;
                w << std::endl;
                for(auto& prop : e.properties)
                {
                    WriteElement(w, prop.Value, depth);
                }

                std::string close = "}";
                close.insert(head.begin(), depth, '\t');
                w << close;
                depth -= 1;
            }
            else
            {
                w << e.value << ",";
            }


        }



	};

}


