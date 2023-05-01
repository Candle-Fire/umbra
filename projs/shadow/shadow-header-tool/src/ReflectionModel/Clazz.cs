namespace shadow_header_tool.ReflectionModel;

public class ClazzRef
{
    public string name;
}

public class Clazz
{
    public string Include;
    
    public string Name;
    public List<Field> Fields = new();
    public List<ClazzRef> Attributes = new();

    public Clazz(string name, string include)
    {
        Include = include;
        Name = name;
    }
    
    public void AddAttribute(ClazzRef attr)
    {
        Attributes.Add(attr);
    }
}

public class Field
{
    public string Name;
    public string Type;
    public List<ClazzRef> Attributes = new();
    
    public Field(string name, string type)
    {
        Name = name;
        Type = type;
    }
    
    public void AddAttribute(ClazzRef attr)
    {
        Attributes.Add(attr);
    }
}