using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class NodeKind
{
    public string text;

    public string Name
    {
        get
        {
            return NodeKind.types.FirstOrDefault(i => i.Value == this).Key;
        }
    }
    
    public static Dictionary<string, NodeKind> types = new()
    {
        {"STRUCT_NODE", new() { text = "STRUCT_NODE" }},
        {"CLASS_NODE", new() { text = "CLASS_NODE" }},
        {"FIELD_NODE", new() { text = "FIELD_NODE" }},
        {"PARAM_NODE", new() { text = "PARAM_NODE" }},
        
        {"INVALID_NODE", new() { text = "INVALID_NODE" }},
        
        {"STRING_LITERAL_NODE", new() { text = "LITERAL_NODE" }},
        {"IDENTIFIER_LITERAL_NODE", new() { text = "IDENTIFIER_LITERAL_NODE" }},
        {"NUMBER_LITERAL_NODE", new() { text = "NUMBER_LITERAL_NODE" }},

        {"CLAZZ_NODE", new() { text = "CLAZZ_NODE" }},
    };
}

public class Node
{
    public NodeKind kind;

    public Node(NodeKind kind)
    {
        this.kind = kind;
    }
}

public class LiteralNode : Node
{
    public Token value;

    public LiteralNode(Token value, NodeKind kind) : base(kind)
    {
        this.value = value;
    }
}

public class AttributeNode : Node
{
    public Token start;
    public List<Node> attribs = new();
    public Token end;

    public AttributeNode(NodeKind kind, Token start, Token end) : base(kind)
    {
        this.start = start;
        this.end = end;
    }
}

public class ClassAttribute : Node
{
    public Token classToken;
    public Node name;
    public Node attribute;

    public ClassAttribute(NodeKind kind, Token classToken, Node name, Node attribute) : base(kind)
    {
        this.classToken = classToken;
        this.name = name;
        this.attribute = attribute;
    }
}
