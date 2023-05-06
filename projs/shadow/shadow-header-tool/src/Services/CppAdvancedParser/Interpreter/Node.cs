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
    
    public static NodeKind COMPILATION_UNIT_NODE = make("COMPILATION_UNIT_NODE");
    
    public static NodeKind USING_NODE = make("USING_NODE");

}

public class Node
{
    public NodeKind kind;

    public Node(NodeKind kind)
    {
        this.kind = kind;
    }
    
    public T to<T>(NodeKind kind) where T : Node
    {
        if(this.kind == kind)
            return (T) this;

        throw new Exception("Can't convert");
    }
    
    public override string ToString() => kind.ToString();
}

public class LiteralNode : Node
{
    public Token ValueToken;

    public LiteralNode(Token valueToken, NodeKind kind) : base(kind)
    {
        this.ValueToken = valueToken;
    }
    public override string ToString() => base.ToString() + " " + ValueToken.value;
    
    public string Value => ValueToken.value;
}

public class AttributeSequenceNode : Node
{
    public Token start;
    public UsingNode? usingNode;
    public List<AttributeNode> attributes = new();
    public Token end;

    public AttributeSequenceNode(Token start, UsingNode? usingNode, List<AttributeNode> attrib, Token end) 
        : base(NodeKind.ATTRIBUTE_SEQ_NODE)
    {
        this.start = start;
        this.usingNode = usingNode;
        this.attributes = attrib;
        this.end = end;
    }
}

public class UsingNode : Node
{
    Token usingToken;
    Token namespaceNameToken;
    public UsingNode(Token usingT, Token namespaceNameToken) : base(NodeKind.USING_NODE)
    {
        this.usingToken = usingT;
        this.namespaceNameToken = namespaceNameToken;
    }
    
    public string NamespaceName => namespaceNameToken.value;
    
    public override string ToString() => base.ToString() + " " + namespaceNameToken.value;
}

public class AttributeNode : Node
{
    public Node nameToken;
    public ParamSeqNode paramSeq;

    public AttributeNode(Node nameToken, ParamSeqNode paramSeq) 
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
    public LiteralNode Value;
    public Token? Comma;

    public ParamNode(LiteralNode value, Token? comma = null) 
        : base(NodeKind.PARAM_NODE)
    {
        this.Value = value;
        this.Comma = comma;
    }
}

public class NamespacedAttributeNode : AttributeNode
{
    public Node namespaceToken;

    public NamespacedAttributeNode(Node namespaceToken, Node nameToken, ParamSeqNode paramSeq) 
        : base(nameToken, paramSeq)
    {
        this.namespaceToken = namespaceToken;
    }
    
    public string Namespace => namespaceToken.to<LiteralNode>(NodeKind.IDENTIFIER_LITERAL_NODE).Value;
}

public class ClassNode : Node
{
    public Token classToken;
    public LiteralNode name;
    public List<AttributeSequenceNode> attributeSeqs = new();

    public ClassNode(Token classToken, LiteralNode name, List<AttributeSequenceNode> attributes) 
        : base(NodeKind.CLASS_NODE)
    {
        this.classToken = classToken;
        this.name = name;
        this.attributeSeqs = attributes;
    }
}

public class NamespaceNode : Node
{
    public Token namespaceToken;
    public LiteralNode name;
    public List<Node> children = new();

    public NamespaceNode(Token namespaceToken, LiteralNode name, List<Node> children) 
        : base(NodeKind.CLASS_NODE)
    {
        this.namespaceToken = namespaceToken;
        this.name = name;
        this.children = children;
    }
}

public class CompilationUnitNode : Node
{
    public List<Node> children = new();

    public CompilationUnitNode(List<Node> children) : base(NodeKind.COMPILATION_UNIT_NODE)
    {
        this.children = children;
    }
}