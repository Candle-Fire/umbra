using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class Parser
{
    public int idx = 0;
    public List<Token> tokens = new();
    
    public List<Token> rest => tokens.Skip(idx).ToList();
    
    public Parser(Lexer l)
    {
        tokens = l.tokens.Where(i=>i.kind != TokenKind.SPACE && i.kind != TokenKind.EOL).ToList();
    }

    public CompilationUnitNode ParseCompilationUnit()
    {
        var nodes = ParseDeclarations();
        return new CompilationUnitNode(nodes);
    }
    
    private List<Node> ParseDeclarations()
    {
        var members = new List<Node>();

        while (idx < tokens.Count)
        {
            var member = ParseDeclaration();
            if(member == null)
                break;
            else
                members.Add(member);
            
        }

        return members;
    }
    
    private Node? ParseDeclaration()
    {
        if (Check(TokenKind.CLAZZ))
            return ParseClass();
        if (Check(TokenKind.NAMESPACE))
            return ParseNamespace();
        if (Check(TokenKind.MACRO))
            return ParseMacro();

        return null;
    }

    private Node ParseMacro()
    {
        var macroToken = Consume(TokenKind.MACRO)!;
        List<Node> rest = new();
        while (Check(TokenKind.IDENTIFIER) || Check(TokenKind.STRING_LITERAL))
        {
            rest.Add(ParseLiteral()!);
        }
        return new MacroNode(macroToken, rest);
    }
    
    private Node ParseClass()
    {
        var classToken = Consume(TokenKind.CLAZZ)!;
        List<AttributeSequenceNode> attributes = new();
        while (Check(TokenKind.ATTRIBUTE_START))
        {
            attributes.Add(ParseAttributeSeq()!);
        }
        var nameNode = ParseQualifiedName();
        
        ParseClassBaseClause();
        
        var members = ParseMemberSpecifications();
        
        return new ClassNode(classToken, nameNode, attributes, members );
    }

    private List<Node> ParseMemberSpecifications()
    {
        List<Node> members = new();

        var depth = 0;
        while (true)
        {
            switch(Peek().kind)
            {
                case { name: "PUBLIC" }:
                case { name: "PRIVATE" }:
                case { name: "PROTECTED" }:
                {
                    Advance();
                    if(Check(TokenKind.COLON))
                        Advance();
                }
                    break;
                case { name: "OPEN_BRACE" }:
                    depth++;
                    Advance();
                    break;
                case { name: "CLOSE_BRACE" }:
                    depth--;
                    Advance();
                    break;
                case { name: "ATTRIBUTE_START" }:
                    members.Add(ParseMemberDeclaration());
                    break;
                default:
                    Advance();
                    break;
            };

            if (depth == 0)
                break;
            
        }

        return members;
    }

    public Node ParseMemberDeclaration()
    {
        List<AttributeSequenceNode> attributes = new();
        if (Check(TokenKind.ATTRIBUTE_START))
            attributes.Add(ParseAttributeSeq());

        var type = ParseQualifiedName();
        var name = ParseLiteral();
        
        if (Check(TokenKind.OPEN_PAREN))
        {
            ConsumeBalancedParens();
            return new MethodNode(name, type, attributes, null);
        }

        return new FieldNode(name, type, attributes);
    }
    
    private void ParseClassBaseClause()
    {
        Consume(TokenKind.COLON);
        while (true)
        {
            var access = Consume(TokenKind.PUBLIC);
            var baseName = ParseQualifiedName();
            if (Check(TokenKind.COMMA))
                Advance();
            else
                break;
        }
    }
    
    private void ConsumeBalancedBraces()
    {
        ConsumeBalancedTags(TokenKind.OPEN_BRACE, TokenKind.CLOSE_BRACE);
    }
    
    private void ConsumeBalancedBrackets()
    {
        ConsumeBalancedTags(TokenKind.OPEN_BRACKET, TokenKind.CLOSE_BRACKET);
    }
    
    private void ConsumeBalancedParens()
    {
        ConsumeBalancedTags(TokenKind.OPEN_PAREN, TokenKind.CLOSE_PAREN);
    }
    
    private void ConsumeBalancedTags(TokenKind open, TokenKind close)
    {
        var depth = 0;
        while (true)
        {
            if (Check(open))
                depth++;
            if (Check(close))
                depth--;
            if (depth == 0)
                break;
            Advance();
        }
    }

    
    private Node ParseNamespace()
    {
        var namespaceToken = Consume(TokenKind.NAMESPACE)!;
        var nameNode = ParseQualifiedName();
        Consume(TokenKind.OPEN_BRACE);
        var members = ParseDeclarations();
        Consume(TokenKind.CLOSE_BRACE);
        return new NamespaceNode(namespaceToken, nameNode, members);
    }
    
    LiteralNode ParseLiteral()
    {
        if (Check(TokenKind.NUMBER_LITERAL))
        {
            var num = Consume(TokenKind.NUMBER_LITERAL)!;
            return new LiteralNode(num, NodeKind.types["NUMBER_LITERAL_NODE"]);
        } 
        if (Check(TokenKind.STRING_LITERAL))
        {
            var str = Consume(TokenKind.STRING_LITERAL)!;
            return new LiteralNode(str, NodeKind.types["STRING_LITERAL_NODE"]);
        }
        if (Check(TokenKind.IDENTIFIER))
        {
            var id = Consume(TokenKind.IDENTIFIER)!;
            return new LiteralNode(id, NodeKind.types["IDENTIFIER_LITERAL_NODE"]);
        }
        
        throw new Exception($"Illegal token at {Peek().ShortString()}");
    }

    QualifiedNameNode ParseQualifiedName()
    {
        var parts = new List<LiteralNode>();
        while (Check(TokenKind.IDENTIFIER))
        {
            parts.Add(ParseLiteral());
            if (Check(TokenKind.COLON))
            {
                Consume(TokenKind.COLON);
                Consume(TokenKind.COLON);
            }
            else
            {
                break;
            }
        }

        return new QualifiedNameNode(parts);
    }
    
    AttributeSequenceNode? ParseAttributeSeq()
    {
        if (!Check(TokenKind.ATTRIBUTE_START))
            return null;
        
        var open = Consume(TokenKind.ATTRIBUTE_START)!;

        List<AttributeNode> attributes = new();
        
        UsingNode? namespaceUsingNode = null;
        if (Check(TokenKind.USING))
        {
            namespaceUsingNode = new UsingNode(
                Consume(TokenKind.USING)!, 
                Consume(TokenKind.IDENTIFIER)!
                );
            Consume(TokenKind.COLON);
        }
        
        while (!Check(TokenKind.ATTRIBUTE_END))
        {
            if (!Check(TokenKind.IDENTIFIER))
                throw new TokenMismatchException(Peek(), TokenKind.IDENTIFIER);
            
            
            Node name;
            Node? namespaceNode = null;
            if (Check(1,TokenKind.COLON) && Check(2, TokenKind.COLON))
            {
                namespaceNode = ParseLiteral();
                Consume(TokenKind.COLON);
                Consume(TokenKind.COLON);
                name = ParseLiteral();
            }
            else
            {
                name = ParseLiteral();
            }

            var paramsSeq = ParseParamsSeq();

            if (namespaceNode is null)
            {
                attributes.Add(new AttributeNode(
                    name, paramsSeq));
            }
            else
            {
                attributes.Add(new NamespacedAttributeNode(
                    namespaceNode, name, paramsSeq));
            }

            if (Check(TokenKind.COMMA))
                Consume(TokenKind.COMMA);
        }
        
        var end = Consume(TokenKind.ATTRIBUTE_END)!;

        return new AttributeSequenceNode(
            open,
            namespaceUsingNode,
            attributes,
            end
        );
    }

    private ParamSeqNode ParseParamsSeq()
    {
        if (Check(TokenKind.OPEN_PAREN))
        {
            var openParen = Consume(TokenKind.OPEN_PAREN)!;
            var args = new List<Node>();
            while (!Check(TokenKind.CLOSE_PAREN))
            {
                if (Check(1,TokenKind.COMMA))
                {
                    args.Add(new ParamNode(ParseLiteral(), Consume(TokenKind.COMMA)));
                }
                else
                {
                    args.Add(new ParamNode(ParseLiteral()));
                }
            }

            var closeParen = Consume(TokenKind.CLOSE_PAREN)!;
            
            return new ParamSeqNode(openParen, args, closeParen);
        }
        
        return null;
    }

    Token Peek(int n) => tokens[idx + n];
    Token Peek() => Peek(0);

    bool Check(int n, TokenKind t) => Peek(n).kind == t;
    bool Check(TokenKind t) => Check(0, t);

    void Advance(int n) => idx += n;
    void Advance() => Advance(1);

    Token? Consume(TokenKind t) {
        if (Check(t))
        {
            var token = Peek();
            Advance();
            return token;
        }
        return null;
    }
}