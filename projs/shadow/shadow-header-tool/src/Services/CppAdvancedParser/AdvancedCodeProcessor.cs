using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.ReflectionModel;
using shadow_header_tool.Services.CppAdvancedParser.Interpreter;
using shadow_header_tool.Services.CppSimpleParser.Lexer;
using Attribute = shadow_header_tool.ReflectionModel.Attribute;

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
        var ast = parser.ParseCompilationUnit();

        var nodeList = ast.children;

        var classes = nodeList.Where(a => a.kind == NodeKind.CLASS_NODE)
            .Select(a => ClazzFromNode(a, new FileInfo(path))).ToList();

        return classes;
    }

    Clazz ClazzFromNode(Node node, FileInfo fileInfo)
    {
        if (node.kind != NodeKind.CLASS_NODE)
        {
            return null;
        }

        var classNode = (ClassNode)node;
        var clazz = new Clazz((classNode.name).ValueToken.value, fileInfo.FullName);

        foreach (var attributeSeq in classNode.attributeSeqs)
        {
            foreach (var attribute in attributeSeq.attributes)
            {
                var nameSpace = attribute is NamespacedAttributeNode nAttribute ? nAttribute.Namespace : "";
                
                clazz.AddAttribute(
                    new Attribute()
                        {
                            Namespace = attributeSeq.usingNode?.NamespaceName ?? nameSpace,
                            Name = attribute.nameToken.to<LiteralNode>(NodeKind.IDENTIFIER_LITERAL_NODE).Value,
                            Parameters = attribute.paramSeq?.paramSeq
                                .Select(p => p.to<ParamNode>(NodeKind.PARAM_NODE).Value.Value)
                                .ToList() ?? new List<string>()
                        }
                );
            }
        }

        return clazz;
    }

    public List<FileInfo> GetFiles()
    {
        throw new NotImplementedException();
    }
}