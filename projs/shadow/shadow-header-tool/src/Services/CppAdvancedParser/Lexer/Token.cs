namespace shadow_header_tool.Services.CppSimpleParser.Lexer;


public class TokenKind
{
    public string text;

    public string Name
    {
        get
        {
            return TokenKind.types.FirstOrDefault(i => i.Value == this).Key;
        }
    }
    
    public static Dictionary<string, TokenKind> types = new()
    {
        {"COLON", new() { text = ":" }},
        
        {"SPACE", new() { text = "SPACE" }},
        {"EOL", new() { text = "EOL" }},
        {"OP", new() { text = "OP" }},

        {"OPEN_BRACKET", new() { text = "[" }},
        {"CLOSE_BRACKET", new() { text = "]" }},
        
        {"OPEN_BRACE", new() { text = "{" }},
        {"CLOSE_BRACE", new() { text = "}" }},
        
        {"OPEN_PAREN", new() { text = "(" }},
        {"CLOSE_PAREN", new() { text = ")" }},
        
        {"ATTRIBUTE_START", new() { text = "ATTRIBUTE_START" }},
        {"ATTRIBUTE_END", new() { text = "ATTRIBUTE_END" }},
        
        {"CLAZZ", new() { text = "class" }},
        {"STRUCT", new() { text = "struct" }},
        {"NAMESPACE", new() { text = "namespace" }},
        {"AUTO", new() { text = "auto" }},
        {"PUBLIC", new() { text = "public" }},
        {"PRIVATE", new() { text = "private" }},
        {"PROTECTED", new() { text = "protected" }},
        
        {"MACRO", new() { text = "#" }},
        {"INCLUDE", new() { text = "include" }},
        {"PRAGMA", new() { text = "pragma" }},

        {"IDENTIFIER", new() { text = "" }},
        {"NUMBER_LITERAL", new() { text = "NUMBER_LITERAL" }},
        {"IDENTIFER_LITERAL", new() { text = "IDENTIFER_LITERAL" }},
        {"STRING_LITERAL", new() { text = "STRING_LITERAL" }},
    };
}

public struct Pos
{
    public int line = 1;
    public int column = 1;

    public Pos()
    {
        
    }
}

public class Token
{
    public string value;
    public TokenKind kind;
    public Pos pos;
    
    
    public Token(Pos p, string value)
    {
        pos = p;
        kind = TokenKind.types.FirstOrDefault(i => i.Value.text == value).Value ?? TokenKind.types["IDENTIFIER"];
        this.value = value;
    }

    public Token(Pos p, TokenKind kind, string value)
    {
        pos = p;
        this.kind = kind;
        this.value = value;
    }
    
    public Token(Pos p, TokenKind kind)
    {
        pos = p;
        this.kind = kind;
        this.value = kind.text;
    }

    public string ShortString()
    {
        return $"{kind.Name} @ L{pos.line} : C{pos.column}";
    }
}