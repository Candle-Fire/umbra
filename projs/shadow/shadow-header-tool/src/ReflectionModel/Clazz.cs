namespace shadow_header_tool.ReflectionModel;

public class ClazzRef
{
    public string name;
    
    public ClazzRef(){}
    
    public ClazzRef(string name)
    {
        this.name = name;
    }
}

public class Attribute
{
    public string Namespace = "";
    public string Name ;
    public List<string> Parameters = new();

    public override string ToString() => $"{Namespace}::{Name}({string.Join(", ", Parameters)})";
}

public class Clazz
{
    public string Include;
    
    public string Name;
    public List<Field> Fields = new();
    public List<Attribute> Attributes = new();

    public Clazz(string name, string include)
    {
        Include = include;
        Name = name;
    }
    
    public void AddAttribute(Attribute attr)
    {
        Attributes.Add(attr);
    }

    public void AddAttributes(List<Attribute> classAttributes)
    {
        this.Attributes.AddRange(classAttributes);
    }

    public void AddField(Field field)
    {
        Fields.Add(field);
    }
}

public class Field
{
    public string Name;
    public string Type;
    public List<Attribute> Attributes = new();
    
    public Field(string name, string type)
    {
        Name = name;
        Type = type;
    }
    
    public void AddAttribute(Attribute attr)
    {
        Attributes.Add(attr);
    }
}