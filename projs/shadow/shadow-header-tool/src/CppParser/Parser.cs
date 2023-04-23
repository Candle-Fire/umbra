using CppAst;

namespace shadow_header_tool.CppProcessor;

public class Parser
{
    public void parse(string path, List<string> includes)
    {
        var options = new CppParserOptions()
        {
            IncludeFolders = {  },
            ParseMacros = false,
            AdditionalArguments =
            {
                "-std=gnu++20"
            }
        };
        options.IncludeFolders.AddRange(includes);
        var a = CppParser.ParseFile(path, options);

        if (a.HasErrors)
        {
            foreach (var message in a.Diagnostics.Messages)
            {
                Console.WriteLine(message.Text);
            }
            
        }
        
        foreach (var aClass in a.Classes)
        {
            Console.WriteLine(aClass.Name);
        }
        
        foreach (var aClass in a.Namespaces)
        {
            Console.WriteLine(aClass.Name);
        }

    }
}