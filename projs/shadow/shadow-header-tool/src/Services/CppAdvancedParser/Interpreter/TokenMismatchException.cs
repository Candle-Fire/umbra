using shadow_header_tool.Services.CppSimpleParser.Lexer;

namespace shadow_header_tool.Services.CppAdvancedParser.Interpreter;

public class TokenMismatchException : Exception
{
    private readonly Token _found;
    private readonly TokenKind _expected;

    public TokenMismatchException(Token found, TokenKind expected)
    {
        _found = found;
        _expected = expected;
    }

    public override string ToString()
    {
        return $"Expected {_expected.name} at {_found.pos} but found {_found.kind.name}";
    }
}