using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class Parser
{
    public int idx = 0;
    public List<Token> tokens = new();
    
    public Parser(Lexer l)
    {
        tokens = l.tokens.Where(i=>i.kind != TokenKind.SPACE && i.kind != TokenKind.EOL).ToList();
    }

    public List<Node> Parse()
    {
        List<Node> nodes = new();
        while (idx < tokens.Count)
        {
            var node = scan();
            if (node is not null)
                nodes.Add(node);
            
        }

        return nodes;
    }

    Node ConsumeLiteral()
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

    Node? ConsumeAttributeSeq()
    {
        if (!Check(TokenKind.ATTRIBUTE_START))
            return null;
        
        var open = Consume(TokenKind.ATTRIBUTE_START)!;

        List<Node> attributes = new();
        
        Token namespaceToken;
        if (Check(TokenKind.USING))
        {
            Consume(TokenKind.USING);
            namespaceToken = Consume(TokenKind.IDENTIFIER)!;
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
                namespaceNode = ConsumeLiteral();
                name = ConsumeLiteral();
            }
            else
            {
                name = ConsumeLiteral();
            }

            var paramsSeq = ConsumeParamsSeq();

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
            attributes,
            end
        );
    }

    private Node ConsumeParamsSeq()
    {
        if (Check(TokenKind.OPEN_PAREN))
        {
            var openParen = Consume(TokenKind.OPEN_PAREN)!;
            var args = new List<Node>();
            while (!Check(TokenKind.CLOSE_PAREN))
            {
                if (Check(1,TokenKind.COMMA))
                {
                    args.Add(new ParamNode(ConsumeLiteral(), Consume(TokenKind.COMMA)));
                }
                else
                {
                    args.Add(new ParamNode(ConsumeLiteral()));
                }
            }

            var closeParen = Consume(TokenKind.CLOSE_PAREN)!;
            
            return new ParamSeqNode(openParen, args, closeParen);
        }
        
        return null;
    }

    Node? scan()
    {
        switch (Peek().kind.Name)
        {
            case "CLAZZ":
            case "STRUCT":
            {
                var classToken = Consume(TokenKind.CLAZZ)!;
                var attr = ConsumeAttributeSeq();
                var nameNode = ConsumeLiteral();
                
                return new ClassNode(classToken, nameNode, new List<Node>{attr} );
            }
            default:
                Advance();
                return null;
        }
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