namespace shadow_header_tool.CmakeLib.model;

public class TargetModel
{
    public string name { get; set; }
    public List<Source> sources { get; set; }
    
    public List<CompileGroup> compileGroups { get; set; }

    public class Source
    {
        public string path { get; set; }
    }
    
    public class CompileGroup
    {
        public List<Include> includes { get; set; }
        
        public class Include
        {
            public string path { get; set; }
        }
    }
}