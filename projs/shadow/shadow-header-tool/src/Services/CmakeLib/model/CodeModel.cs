namespace shadow_header_tool.Services.CmakeLib.model;

public class CodeModel
{
    public List<Configuration> configurations { get; set; } = new();
    
    public class Configuration
    {
        public List<Target> targets { get; set; } = new();
    }

    public class Target
    {
        public string name { get; set; }
        public string jsonFile { get; set; }

        public Target(string name, string jsonFile)
        {
            this.name = name;
            this.jsonFile = jsonFile;
        }
    }
}

