using shadow_header_tool.ReflectionModel;

namespace shadow_header_tool.OutputWriter;

public interface ICppReflectionDataWriter
{
    /// <summary>
    /// Function to write a cpp file that has all the reflection data
    /// </summary>
    /// <param name="path">Path of the target cpp file</param>
    /// <param name="classes">List of all of the classes that should be written</param>
    void Write(string path, List<Clazz> classes);
}