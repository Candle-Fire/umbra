namespace shadow_header_tool.OutputWriter;

public class FormattedPrinter
{
    private readonly StreamWriter _writer;
    int indent = 0;

    public FormattedPrinter(StreamWriter writer)
    {
        _writer = writer;
    }
    
    public Indent AddLevel()
    {
        return new Indent(this);
    }
    
    public void WriteLine(string line)
    {
        _writer.WriteLine(new string(' ', indent * 4) + line);
    }
    
    public void Close()
    {
        _writer.Close();
    }

    public class Indent : IDisposable
    {
        private FormattedPrinter parent;
        public Indent(FormattedPrinter parent)
        {
            this.parent = parent;
            parent.indent++;
        }

        public void Dispose()
        {
            parent.indent--;
        }
    }


}