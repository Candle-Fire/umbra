namespace shadow_header_tool.AssertHelpers;

public static class Asserts
{
    public static void Assert(Func<bool> test, string message)
    {
        if (!test.Invoke())
        {
            throw new Exception(message);
        }
    }
    
    public static void Assert(bool test, string message)
    {
        if (!test)
        {
            throw new Exception(message);
        }
    }
}