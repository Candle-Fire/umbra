using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class NodeKind
{
    public string name;

    public string Name => NodeKind.types.FirstOrDefault(i => i.Value == this).Key;
    public override string ToString() => name;

    
    //STATIC PART
    
    public static Dictionary<string, NodeKind> types = new();

    private static NodeKind make(string name, string s)
    {
        var a = new NodeKind() { name = name };
        types.Add(name, a);
        return a;
    }

    private static NodeKind make(string name)
    {
        return make(name, name);
    }

    
        public static NodeKind CLASS_NODE = make("CLASS_NODE");
        public static NodeKind STRUCT_NODE = make("STRUCT_NODE");
        public static NodeKind FIELD_NODE = make("FIELD_NODE");

        public static NodeKind PARAM_SEQ_NODE = make("PARAM_SEQ_NODE");
        public static NodeKind PARAM_NODE = make("PARAM_NODE");

        public static NodeKind INVALID_NODE = make("INVALID_NODE");

        public static NodeKind STRING_LITERAL_NODE = make("STRING_LITERAL_NODE");
        public static NodeKind IDENTIFIER_LITERAL_NODE = make("IDENTIFIER_LITERAL_NODE");
        public static NodeKind NUMBER_LITERAL_NODE = make("NUMBER_LITERAL_NODE");

        public static NodeKind ATTRIBUTE_SEQ_NODE = make("ATTRIBUTE_SEQ_NODE");
        public static NodeKind ATTRIBUTE_NODE = make("ATTRIBUTE_NODE");

}

public class Node
{
    public NodeKind kind;

    public Node(NodeKind kind)
    {
        this.kind = kind;
    }
    
    public override string ToString() => kind.ToString();
}

public class LiteralNode : Node
{
    public Token value;

    public LiteralNode(Token value, NodeKind kind) : base(kind)
    {
        this.value = value;
    }
    public override string ToString() => base.ToString() + " " + value.value;
}

public class AttributeSequenceNode : Node
{
    public Token start;
    public List<Node> attributes = new();
    public Token end;

    public AttributeSequenceNode(Token start, List<Node> attrib, Token end) 
        : base(NodeKind.ATTRIBUTE_SEQ_NODE)
    {
        this.start = start;
        this.attributes = attrib;
        this.end = end;
    }
}

public class AttributeNode : Node
{
    public Node nameToken;
    
    public Node paramSeq;

    public AttributeNode(Node nameToken, Node paramSeq) 
        : base(NodeKind.ATTRIBUTE_NODE)
    {
        this.nameToken = nameToken;
        this.paramSeq = paramSeq;
    }
}

public class ParamSeqNode : Node
{
    public Token open;
    public List<Node> paramSeq = new();
    public Token close;
    
    public ParamSeqNode(Token open, List<Node> paramSeq, Token close) 
        : base(NodeKind.PARAM_SEQ_NODE)
    {
        this.open = open;
        this.paramSeq = paramSeq;
        this.close = close;
    }
}

public class ParamNode : Node
{
    public Node Value;
    public Token? Comma;

    public ParamNode(Node value, Token? comma = null) 
        : base(NodeKind.PARAM_NODE)
    {
        this.Value = value;
        this.Comma = comma;
    }
}

public class NamespacedAttributeNode : AttributeNode
{
    public Node namespaceToken;

    public NamespacedAttributeNode(Node namespaceToken, Node nameToken, Node paramSeq) 
        : base(nameToken, paramSeq)
    {
        this.namespaceToken = namespaceToken;
    }
}

public class ClassNode : Node
{
    public Token classToken;
    public Node name;
    public List<Node> attributeSeqs = new();

    public ClassNode(Token classToken, Node name, List<Node> attributes) 
        : base(NodeKind.CLASS_NODE)
    {
        this.classToken = classToken;
        this.name = name;
        this.attributeSeqs = attributes;
    }
}
