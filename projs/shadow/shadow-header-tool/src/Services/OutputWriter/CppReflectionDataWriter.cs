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

        var w = new FormattedPrinter(writer);
        
        w.WriteLine("#include <reflection.h>");
        w.WriteLine("using namespace SH::Reflection;");
        w.WriteLine("");

        foreach (var clazz in classes)
        {
            var reflectAttr = clazz.Attributes.Find(i => i.name == "SH::Reflect");
            if (reflectAttr is not null)
                PrintClassReflection(w, clazz);
        }
        
        w.Close();
    }

    private static void PrintClassReflection(FormattedPrinter w, Clazz clazz)
    {
        var fields = clazz.Fields.Where(i=>i.Attributes.Any(i=>i.name == "SH::Reflect")).ToList();
        
        w.WriteLine($"#include <{clazz.Include}>");
        w.WriteLine($"template<> Class const *SH::Reflection::GetClass(ClassTag<{clazz.Name}>){{");

        using (w.AddLevel())
        {
            w.WriteLine($"static detail::ClassStorage<{clazz.Name}, {fields.Count}, 0, 0> reflected([](auto self) {{");

            using (w.AddLevel())
            {
                int id = 0;
                foreach (var field in fields)
                {
                    w.WriteLine($"self->fields[{id}].type = GetType<{field.Type}>();");
                    w.WriteLine($"self->fields[{id}].name = \"{field.Name}\";");
                    w.WriteLine($"self->fields[{id}].offset = offsetof({clazz.Name}, {field.Name});");
                    id++;
                }
            }

            w.WriteLine("});");

            w.WriteLine($"static Class const clazz(\"{clazz.Name}\", sizeof(User),reflected.fields, reflected.numFields);");
            w.WriteLine("return &clazz;");
        }

        w.WriteLine("}");

        w.WriteLine($"const Class *{clazz.Name}::GetClass() const {{ return SH::Reflection::GetClass(ClassTag<{clazz.Name}>{{}}); }}");
        w.WriteLine("");
    }
}