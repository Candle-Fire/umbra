namespace shadow_header_tool.ReflectionModel;

public class Clazz
{
    public string Include;
    
    public string Name;
    public List<Field> Fields = new();

    public Clazz(string include, string name)
    {
        Include = include;
        Name = name;
    }
}

public class Field
{
    public string Name;
    public string Type;
    
    public Field(string name, string type)
    {
        Name = name;
        Type = type;
    }
}