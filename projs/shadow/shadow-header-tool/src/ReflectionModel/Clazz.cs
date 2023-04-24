namespace shadow_header_tool.ReflectionModel;

public class Clazz
{
    public string Include;
    
    public string Name;
    public List<Field> Fields = new();
}

public class Field
{
    public string Name;
    public string Type;
}