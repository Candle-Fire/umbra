
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.ReflectionModel;
using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser;

public class AdvancedCodeProcessor : ICodeProcessor
{
    List<string> _paths = new();
    List<string> _includes = new();
    
    public void AddSourceFiles(List<string> paths, List<string> includes)
    {
        _paths = paths;
        _includes = includes;
    }

    public List<Clazz> Process()
    {
        var path = "P:\\_Projects\\umbra\\umbra\\projs\\shadow\\shadow-engine\\shadow-entity\\inc\\entities\\Position.h";
        
        Console.WriteLine(path);
        var lexer = new Lexer();
        lexer.LoadFile(new FileInfo(path));
        

        return new List<Clazz>();
    }

    public List<FileInfo> GetFiles()
    {
        throw new NotImplementedException();
    }
}