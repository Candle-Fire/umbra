module;

module Shadow.FileFormat:SFFWriter;

import <string>;
import <iostream>;
import <fstream>;
import <string>;
import shadow_utils;

import :SFFElement;
import :SFFVersion;


namespace Shadow::SFF {

    /*
    void SFFWriter::WriteFile(SFFElement& root, std::string path)
    {
        
        try
        {
            std::ofstream writer;
            writer.open(path);
            writer.WriteLine("ShadowFileFormat_1_0_0");

            int depth = 0;
            WriteElement(writer, root, ref depth);

            writer.Close();
        }
        catch (Exception e) {

        }
        
    }

    void SFFWriter::WriteElement(std::ofstream w, SFFElement& e, int& depth)
    {
        
        string head = (e.name + (e.isBlock ? ":{" : ":"));
        head = head.PadLeft(depth + head.Length, '\t');
        w.Write(head);
        if (e.isBlock)
        {
            depth += 1;
            w.WriteLine();
            foreach(var property in e.properties)
            {
                WriteElement(w, property.Value, ref depth);
            }
            w.WriteLine(("}").PadLeft(depth, '\t'));
            depth -= 1;
        }
        else
        {
            w.WriteLine(e.value + ",");
        }
        
    }
*/

}



