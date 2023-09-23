namespace shadow_header_tool.CmakeLib.model;

public class TargetModel
{
    public string name { get; set; }
    public List<Source> sources { get; set; } = new();
    public List<CompileGroup> compileGroups { get; set; } = new();

    public TargetModel(string name)
    {
        this.name = name;
    }

    public class Source
    {
        public string path { get; set; }

        public Source(string path)
        {
            this.path = path;
        }
    }
    
    public class CompileGroup
    {
        public List<Include> includes { get; set; } = new();
        
        public class Include
        {
            public string path { get; set; }

            public Include(string path)
            {
                this.path = path;
            }
        }
    }
}