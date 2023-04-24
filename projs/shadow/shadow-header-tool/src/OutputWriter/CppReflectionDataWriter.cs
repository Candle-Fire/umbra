using shadow_header_tool.ReflectionModel;

namespace shadow_header_tool.OutputWriter;

/* Example of the output:
 *  template<>
 *  Class const *GetClass(ClassTag<User>) {
 *
 *      static detail::ClassStorage<User, 3, 0, 0> reflected([](auto self) {
 *          self->fields[0].type = GetType<std::uint64_t>();
 *          self->fields[0].name = "id";
 *          self->fields[0].offset = offsetof(User, id);
 *          self->fields[1].type = GetType<std::string>();
 *          self->fields[1].name = "name";
 *          self->fields[1].offset = offsetof(User, name);
 *          self->fields[2].type = GetType<std::vector<std::string>>();
 *          self->fields[2].name = "pets";
 *          self->fields[2].offset = offsetof(User, pets);
 *      });
 *      static Class clazz(reflected.fields, reflected.numFields);
 *
 *      return &clazz;
 *  } 
 */

public interface ICppReflectionDataWriter
{
    /// <summary>
    /// Function to write a cpp file that has all the reflection data
    /// </summary>
    /// <param name="path">Path of the target cpp file</param>
    /// <param name="classes">List of all of the classes that should be written</param>
    void Write(string path, List<Clazz> classes);
}

public class CppReflectionDataWriter : ICppReflectionDataWriter
{
    /// <summary>
    /// Function to write a cpp file that has all the reflection data
    /// </summary>
    /// <param name="path">Path of the target cpp file</param>
    /// <param name="classes">List of all of the classes that should be written</param>
    public void Write(string path, List<Clazz> classes)
    {
        FileStream fileStream = new(path, FileMode.Create);
        StreamWriter writer = new(fileStream);
        
        writer.WriteLine("#include <reflection.h>");
        writer.WriteLine();

        foreach (var clazz in classes)
        {
            writer.WriteLine($"#include <{clazz.Include}>");
            writer.WriteLine($"template<> Class const *GetClass(ClassTag<{clazz.Name}>){{");
            
            writer.WriteLine($"static detail::ClassStorage<User, {clazz.Fields.Count}, 0, 0> reflected([](auto self) {{");

            for (int i = 0; i < clazz.Fields.Count; i++)
            {
                var field = clazz.Fields[i];
                writer.WriteLine($"self->fields[{i}].type = GetType<{field.Type}>();");
                writer.WriteLine($"self->fields[{i}].name = \"{field.Name}\";");
                writer.WriteLine($"self->fields[{i}].offset = offsetof({clazz.Name}, {field.Name});");
            }
            
            writer.WriteLine("});");
            
            writer.WriteLine("static Class const clazz(reflected.fields, reflected.numFields);");
            writer.WriteLine("return &clazz;");

            writer.WriteLine("}");
        }
        
        writer.Close();
    }
}