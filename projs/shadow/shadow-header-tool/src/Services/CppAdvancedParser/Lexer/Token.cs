namespace shadow_header_tool.Services.CppSimpleParser.Lexer;


public class TokenKind
{
    public string text;

    public string name;

    public string Name
    {
        get
        {
            return TokenKind.types.FirstOrDefault(i => i.Value == this).Key;
        }
    }
    
    public static Dictionary<string, TokenKind> types = new();
    
    public static TokenKind COLON = make("COLON", ":");
    
    public static TokenKind SPACE = make("SPACE", "SPACE");
    public static TokenKind EOL = make("EOL", "EOL");
    public static TokenKind OP = make("OP", "OP");
    
    public static TokenKind OPEN_BRACKET = make("OPEN_BRACKET", "[");
    public static TokenKind CLOSE_BRACKET = make("CLOSE_BRACKET", "]");
    
    public static TokenKind OPEN_BRACE = make("OPEN_BRACE", "{");
    public static TokenKind CLOSE_BRACE = make("CLOSE_BRACE", "}");
    
    public static TokenKind OPEN_PAREN = make("OPEN_PAREN", "(");
    public static TokenKind CLOSE_PAREN = make("CLOSE_PAREN", ")");
    
    public static TokenKind COMMA = make("COMMA", ",");
    public static TokenKind SEMICOLON = make("SEMICOLON", ";");
    
    public static TokenKind ATTRIBUTE_START = make("ATTRIBUTE_START");
    public static TokenKind ATTRIBUTE_END = make("ATTRIBUTE_END");
    
    public static TokenKind CLAZZ = make("CLAZZ", "class");
    public static TokenKind STRUCT = make("STRUCT", "struct");
    public static TokenKind NAMESPACE = make("NAMESPACE", "namespace");
    public static TokenKind AUTO = make("AUTO", "auto");
    public static TokenKind PUBLIC = make("PUBLIC", "public");
    public static TokenKind PRIVATE = make("PRIVATE");
    public static TokenKind PROTECTED = make("PROTECTED");
    public static TokenKind TEMPLATE = make("TEMPLATE");

    public static TokenKind USING = make("USING", "using");
    
    public static TokenKind MACRO = make("MACRO", "#");
    //public static TokenKind INCLUDE = make("INCLUDE", "include");
    //public static TokenKind PRAGMA = make("PRAGMA", "pragma");
    
    public static TokenKind IDENTIFIER = make("IDENTIFIER");
    public static TokenKind NUMBER_LITERAL = make("NUMBER_LITERAL");
    public static TokenKind STRING_LITERAL = make("STRING_LITERAL");
    
    
    private static TokenKind make(string name, string s)
    {
        var a = new TokenKind() { name = name, text = s };
        types.Add(name, a);
        return a;
    }
    
    private static TokenKind make(string name)
    {
        return make(name, name);
    }
}

public struct Pos
{
    public int line = 1;
    public int column = 1;

    public Pos()
    {
        
    }

    public override string ToString()
    {
        return "L" + line + ":C" + column;
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
        kind = TokenKind.types.FirstOrDefault(i => i.Value.text == value).Value ??
               TokenKind.types["IDENTIFIER"];
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

    public override string ToString()
    {
        return $"{kind.Name}({value}) @ {pos}";
    }
}