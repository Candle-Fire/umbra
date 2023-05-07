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

    public static NodeKind NAMESPACE_NODE = make("NAMESPACE_NODE");
    public static NodeKind CLASS_NODE = make("CLASS_NODE");
    public static NodeKind STRUCT_NODE = make("STRUCT_NODE");
    
    public static NodeKind FIELD_NODE = make("FIELD_NODE");
    public static NodeKind METHOD_NODE = make("METHOD_NODE");

    public static NodeKind PARAM_SEQ_NODE = make("PARAM_SEQ_NODE");
    public static NodeKind PARAM_NODE = make("PARAM_NODE");

    public static NodeKind INVALID_NODE = make("INVALID_NODE");

    public static NodeKind STRING_LITERAL_NODE = make("STRING_LITERAL_NODE");
    public static NodeKind IDENTIFIER_LITERAL_NODE = make("IDENTIFIER_LITERAL_NODE");
    public static NodeKind NUMBER_LITERAL_NODE = make("NUMBER_LITERAL_NODE");
    public static NodeKind QUALIFIED_NAME_NODE = make("QUALIFIED_NAME_NODE");

    public static NodeKind ATTRIBUTE_SEQ_NODE = make("ATTRIBUTE_SEQ_NODE");
    public static NodeKind ATTRIBUTE_NODE = make("ATTRIBUTE_NODE");
    
    public static NodeKind COMPILATION_UNIT_NODE = make("COMPILATION_UNIT_NODE");
    
    public static NodeKind USING_NODE = make("USING_NODE");
    
    public static NodeKind MACRO_NODE = make("MACRO_NODE");
    
    

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

    public virtual List<Node> Walk() => new List<Node>();

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

public class QualifiedNameNode : Node
{
    public List<LiteralNode> parts = new();

    public QualifiedNameNode(List<LiteralNode> parts) : base(NodeKind.QUALIFIED_NAME_NODE)
    {
        this.parts = parts;
    }
    
    public override string ToString() => base.ToString() + " " + string.Join("::", parts.Select(i => i.Value));
    
    public string Value => string.Join("::", parts.Select(i => i.Value));
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

public class FieldNode : Node
{
    public LiteralNode nameToken;
    public QualifiedNameNode typeToken;
    public List<AttributeSequenceNode> attributes;

    public FieldNode(LiteralNode nameToken, QualifiedNameNode typeToken, List<AttributeSequenceNode> attribs) 
        : base(NodeKind.FIELD_NODE)
    {
        this.nameToken = nameToken;
        this.typeToken = typeToken;
        this.attributes = attribs;
    }
}

public class MethodNode : Node
{
    public LiteralNode nameToken;
    public QualifiedNameNode returnTypeToken;
    public List<AttributeSequenceNode> attributes;
    public ParamSeqNode paramSeq;

    public MethodNode(LiteralNode nameToken,
        QualifiedNameNode returnTypeToken,
        List<AttributeSequenceNode> attribs,
        ParamSeqNode paramSeq) 
        : base(NodeKind.METHOD_NODE)
    {
        this.nameToken = nameToken;
        this.returnTypeToken = returnTypeToken;
        this.attributes = attribs;
        this.paramSeq = paramSeq;
    }
}

public class ClassNode : Node
{
    public Token classToken;
    public QualifiedNameNode name;
    public List<AttributeSequenceNode> attributeSeqs = new();
    public List<Node> children = new();

    public ClassNode(Token classToken,
        QualifiedNameNode name,
        List<AttributeSequenceNode> attributes,
        List<Node> children) 
        : base(NodeKind.CLASS_NODE)
    {
        this.classToken = classToken;
        this.name = name;
        this.attributeSeqs = attributes;
        this.children = children;
    }
}

public class NamespaceNode : Node
{
    public Token namespaceToken;
    public QualifiedNameNode name;
    public List<Node> children = new();

    public NamespaceNode(Token namespaceToken, QualifiedNameNode name, List<Node> children) 
        : base(NodeKind.NAMESPACE_NODE)
    {
        this.namespaceToken = namespaceToken;
        this.name = name;
        this.children = children;
    }

    public override List<Node> Walk() => children;
}

public class CompilationUnitNode : Node
{
    public List<Node> children = new();

    public CompilationUnitNode(List<Node> children) : base(NodeKind.COMPILATION_UNIT_NODE)
    {
        this.children = children;
    }
    
    public override List<Node> Walk() => children;
}

public class MacroNode : Node
{
    public Token hashToken;
    public List<Node> tokens = new();

    public MacroNode(Token hashToken, List<Node> rest) : base(NodeKind.MACRO_NODE)
    {
        this.hashToken = hashToken;
        this.tokens = rest;
    }
}