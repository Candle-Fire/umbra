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

        var classes =
            ast.children
                .Select(i=>(parents: new List<Node>(){ast}, node: i))
                .SelectMany(i=>i.node.Walk().Select(a =>
                    {
                        var parents = new List<Node>();
                        parents.AddRange(i.parents);
                        parents.Add(i.node);
                        return (parents: parents, node: a);
                    }
                ))
                .Where(a => a.node.kind == NodeKind.CLASS_NODE)
                .Select(a => ClazzFromNode(a, new FileInfo(path))).ToList();

        return classes;
    }

    Clazz ClazzFromNode((List<Node>,Node) node, FileInfo fileInfo)
    {
        if (node.Item2.kind != NodeKind.CLASS_NODE)
        {
            return null;
        }

        var namespaceName = GetNamespaceName(node.Item1);
        var classNode = (ClassNode)node.Item2;
        var name = classNode.name.Value;
        if(namespaceName is not null)
            name = namespaceName + "::" + name;
        
        var clazz = new Clazz(name, fileInfo.FullName);
        
        clazz.AddAttributes(GetAttributeList(classNode.attributeSeqs));

        foreach (var child in classNode.children)
        {
            if (child.kind == NodeKind.FIELD_NODE)
            {
                var fieldNode = (FieldNode) child;
                var field = new Field(fieldNode.nameToken.Value, fieldNode.typeToken.Value);
                field.Attributes.AddRange(GetAttributeList(fieldNode.attributes));
                clazz.AddField(field);
            }
        }
        
        return clazz;
    }

    private static List<Attribute> GetAttributeList(List<AttributeSequenceNode> nodes)
    {
        List<Attribute> attributes = new();
        foreach (var attributeSeq in nodes)
        {
            foreach (var attribute in attributeSeq.attributes)
            {
                var nameSpace = attribute is NamespacedAttributeNode nAttribute ? nAttribute.Namespace : "";

                attributes.Add(
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
        
        return attributes;
    }

    string? GetNamespaceName(List<Node> parents)
    {
        var namespaceNode = parents
            .Where(i => i.kind == NodeKind.NAMESPACE_NODE)
            .Select(i=>((NamespaceNode)i).name.Value).ToList();
        if (namespaceNode.Count == 0) return null;
        
        var namespaceName = string.Join("::", namespaceNode);
        return namespaceName;
    }
    
    public List<FileInfo> GetFiles()
    {
        throw new NotImplementedException();
    }
}