using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class Interpreter
{
    public int idx = 0;
    public List<Token> tokens = new();
    
    public Interpreter(Lexer l)
    {
        tokens = l.tokens;
    }


    Node literal()
    {
        if (Check(TokenKind.types["NUMBER_LITERAL"]))
        {
            var num = Consume(TokenKind.types["NUMBER_LITERAL"])!;
            return new LiteralNode(num, NodeKind.types["NUMBER_LITERAL_NODE"]);
        } 
        if (Check(TokenKind.types["STRING_LITERAL"]))
        {
            var str = Consume(TokenKind.types["STRING_LITERAL"])!;
            return new LiteralNode(str, NodeKind.types["STRING_LITERAL_NODE"]);
        }
        if (Check(TokenKind.types["IDENTIFIER_LITERAL"]))
        {
            var id = Consume(TokenKind.types["IDENTIFIER_LITERAL"])!;
            return new LiteralNode(id, NodeKind.types["IDENTIFIER_LITERAL_NODE"]);
        }
        
        throw new Exception($"Illegal token at {Peek().ShortString()}");
    }
/*
    Node scan()
    {
        switch (peek().kind.Name)
        {
            case "CLAZZ":
            case "STRUCT":
                
        }
    }
*/
    Token Peek(int n) => tokens[idx + n];
    Token Peek() => Peek(0);

    bool Check(int n, TokenKind t) => Peek(n).kind == t;
    bool Check(TokenKind t) => Check(0, t);

    void Advance(int n) => idx += n;
    void Advance() => Advance(0);

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