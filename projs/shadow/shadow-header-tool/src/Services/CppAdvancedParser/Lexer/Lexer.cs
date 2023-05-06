namespace shadow_header_tool.Services.CppSimpleParser.Lexer;

public class Lexer
{
    public List<Token> tokens = new();
    private string buffer;
    private int idx;
    private Pos pos = new();

    public void LoadFile(FileInfo file)
    {
        StreamReader reader = new(file.FullName);
        buffer = reader.ReadToEnd().Replace('\r', ' ') + '\0';
        while (true)
        {
            if(idx >= buffer.Length) break;
            var token = scan_token();
            tokens.Add(token);
            
            Console.WriteLine($"Token: {token.kind.Name} at: L{token.pos.line}C{token.pos.column} {token.value}");
            advance();
        }
    }

    char peek(int n) => buffer[idx + n];
    char peek() => peek(0);

    void advance(int n)
    {
        pos.column += n;
        idx += n;
    }

    void advance() => advance(1);
    bool check(int n, char c) => peek(n) == c;
    bool check(char c) => check(0, c);

    Token scan_token()
    {
        switch (peek())
        {
            case ' ':
            case '\t': return new Token(pos, TokenKind.SPACE);
            case '\n':
                var tok = new Token(pos, TokenKind.EOL);
                pos.line += 1;
                pos.column = 1;
                return tok;
            case '[':
                if (check(1, '[')) {
                    advance();
                    return new Token(pos, TokenKind.ATTRIBUTE_START);
                }
                return new Token(pos, TokenKind.OPEN_BRACKET);
            case ']': 
                if (check(1, ']')) {
                    advance();
                    return new Token(pos, TokenKind.ATTRIBUTE_END);
                }
                return new Token(pos, TokenKind.CLOSE_BRACKET);
            case '{': return new Token(pos, TokenKind.OPEN_BRACE);
            case '}': return new Token(pos, TokenKind.CLOSE_BRACE);
            case '(': return new Token(pos, TokenKind.OPEN_PAREN);
            case ')': return new Token(pos, TokenKind.CLOSE_PAREN);
            case ':': return new Token(pos, TokenKind.COLON);
            case '\"': return concat_string();
            default:
                if (char.IsNumber(peek()))
                {
                    var number = concat_number();
                    return number;
                }
                else if (char.IsLetter(peek()))
                {
                    var identifier = concat_identifier();
                    return identifier;
                }
                else
                {
                    return new Token(pos, peek()+"");
                }
        }

        //return null;
    }

    Token concat_identifier()
    {
        string str = "";
        while (char.IsLetterOrDigit(peek()) || check('_'))
        {
            str += peek();
            advance();
        }

        idx--;
        pos.column--;
        return new Token(pos, str);
    }

    Token concat_number()
    {
        string str = "";
        while (char.IsNumber(peek()))
        {
            str += peek();
            advance();
        }
        
        idx--;
        pos.column--;
        return new Token(pos, str);
    }

    Token concat_string()
    {
        string str = "";
        if (!check('\"'))
        {
            throw new Exception($"Error: Token '\"' expected at {pos} got {peek()}");
        }

        advance(); // Eat "
        while (!check('\"'))
        {
            str += peek();
            advance();
        }

        //advance(); // Eat "
        return new Token(pos, TokenKind.STRING_LITERAL, str);
    }
}