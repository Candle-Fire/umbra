
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.ReflectionModel;
using shadow_header_tool.Services.CppAdvancedParser.Interpreter;
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
        var path = "P:\\_Projects\\umbra\\umbra\\projs\\shadow\\shadow-header-tool\\src\\Services\\CppAdvancedParser\\test_files\\test.cpp";
        
        Console.WriteLine(path);
        var lexer = new Lexer();
        lexer.LoadFile(new FileInfo(path));
        
        var parser = new Parser(lexer);
        var ast = parser.Parse();

        return new List<Clazz>();
    }

    public List<FileInfo> GetFiles()
    {
        throw new NotImplementedException();
    }
}