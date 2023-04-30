using shadow_header_tool.ReflectionModel;

namespace shadow_header_tool.CppSimpleParser;

public interface IParser
{
    void AddSourceFiles(List<string> paths, List<string> includes);
    List<Clazz> Process();
    List<FileInfo> GetFiles();
}