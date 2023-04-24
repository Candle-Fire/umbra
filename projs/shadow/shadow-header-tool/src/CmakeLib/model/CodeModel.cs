namespace shadow_header_tool.CmakeLib.model;

public class CodeModel
{
    public List<Configuration> configurations { get; set; }
    
    public class Configuration
    {
        public List<Target> targets { get; set; }
    }

    public class Target
    {
        public string name { get; set; }
        public string jsonFile { get; set; }
    }
}

