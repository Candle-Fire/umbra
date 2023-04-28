namespace shadow_header_tool.CmakeLib;

public interface ICodeLoader
{
    List<string> GatherSourceFiles(string cmake, string project, List<string> exclude);
    List<string> getIncludeDirs(string project);
}