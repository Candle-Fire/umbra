using System.Text.RegularExpressions;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.ReflectionModel;
using shadow_header_tool.Services.CppAdvancedParser.Interpreter;
using shadow_header_tool.Services.CppSimpleParser;
using shadow_header_tool.Services.CppSimpleParser.Lexer;
using Attribute = shadow_header_tool.ReflectionModel.Attribute;

namespace shadow_header_tool.Services.CppAdvancedParser;

public class AdvancedCodeProcessor : ICodeProcessor
{
    List<string> _paths = new();
    List<string> _includes = new();

    //The list of files that are included in the source files
    HashSet<string> _files = new(); 
    
    private FileCache _fileCache;

    string includePattern = @"#include [<\""](?<path>[\w\/\\\.]+)[>\""]";

    public AdvancedCodeProcessor(FileCache cache)
    {
        _fileCache = cache;
    }
    
    public List<FileInfo> GetFiles()
    {
        return _files.Select(f => new FileInfo(f)).ToList();
    }
    
    public void AddSourceFiles(List<string> paths, List<string> includes)
    {
        _paths = paths;
        _includes = includes;
        
        var inc = includes.Select(i => new DirectoryInfo(i));
        
        foreach (var path in paths)
        {
            if (!path.EndsWith(".cpp") && !path.EndsWith(".h"))
            {
                continue;
            }
            
            var fileInfo = new FileInfo(path);
            if(!fileInfo.Exists)
            {
                continue;
            }
            
            var file = _fileCache.ReadFile(path);

            foreach (Match m in Regex.Matches(file.content, includePattern, RegexOptions.Multiline))
            {
            
                var incFile = inc
                    .Select(i => new FileInclude(
                        file: new FileInfo(i.FullName + "/" + m.Groups["path"].Value),
                        path: m.Groups["path"].Value
                    ))
                    .ToList()
                    .Find(f => f.File.Exists);
                if (incFile != null)
                {
                    //Console.WriteLine($"{m.Value, -60} is at: {incFile.File.FullName, 10}");
                    _files.Add(incFile.File.FullName);
                }
                else
                {
                    //Console.WriteLine($"{m.Value, -60} is not found");
                }
            }
        }
    }

    public List<Clazz> Process()
    {
        List<Clazz> classes = new();
        
        foreach (var file in _files)
        {


            //var file = "P:\\_Projects\\umbra\\umbra\\projs\\shadow\\shadow-header-tool\\src\\Services\\CppAdvancedParser\\test_files\\test.cpp";

            Console.WriteLine(file);
            var lexer = new Lexer();
            lexer.LoadFile(new FileInfo(file));

            var parser = new Parser(lexer);
            var ast = parser.ParseCompilationUnit();

            var nodeList = ast.children;

            var new_classes = new []{(parents: new List<Node>() { }, node: ast)}
                    //ast.children
                    //.Select(i => (parents: new List<Node>() { ast }, node: i))
                    .SelectMany(i => i.node.Walk().Select(a =>
                        {
                            var parents = new List<Node>();
                            parents.AddRange(i.parents);
                            parents.Add(i.node);
                            return (parents: parents, node: a);
                        }
                    ))
                    .SelectMany(i => i.node.Walk().Select(a =>
                        {
                            var parents = new List<Node>();
                            parents.AddRange(i.parents);
                            parents.Add(i.node);
                            return (parents: parents, node: a);
                        }
                    ))
                    .Where(a => a.node.kind == NodeKind.CLASS_NODE)
                    .Select(a => ClazzFromNode(a, new FileInfo(file))).ToList();
            
            classes.AddRange(new_classes);
        }

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
}