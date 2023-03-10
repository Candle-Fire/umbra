#include "SFFWriter.h"


namespace Shadow::SFF {

    void SFFWriter::WriteFile(SFFElement& root, std::string path)
    {
        std::ofstream writer(path);
        writer << "ShadowFileFormat_1_0_0" << std::endl;

        int depth = 0;
        WriteElement(writer, root, depth);

        writer.flush();
    }

    void SFFWriter::WriteElement(std::ostream& w, SFFElement& e, int &depth)
    {
            std::string head = (e.name + (e.isBlock ? ":{" : ":"));
            //head = head.PadLeft(depth + head.Length, '\t');
            head.insert(head.begin(), depth, '\t');
            w << head << std::endl;

            if (e.isBlock)
            {
                depth += 1;
                w << std::endl;
                for(auto& prop : e.children)
                {
                    WriteElement(w, *prop.second, depth);
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

}


