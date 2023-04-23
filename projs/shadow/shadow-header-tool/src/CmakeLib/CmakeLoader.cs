using System.Text.Json;
using shadow_header_tool.CmakeLib.model;

namespace shadow_header_tool.CmakeLib;

public class CmakeLoader
{
    public static string cmakeRoot = "./cmake-build-debug/";
    public static string cmakeFileAPI = $"{cmakeRoot}.cmake/api/v1/reply/";

    public List<string> getFiles(string project)
    {
        var a = LoadCoreModel();
        var targetJson = a.configurations[0].targets.Find(i => i.name == project).jsonFile;

        var target = LoadTargetModel(targetJson);

        return target.sources.Select(s=>s.path).ToList();
    }
    
    public List<string> getIncludeDirs(string project)
    {
        var a = LoadCoreModel();
        var targetJson = a.configurations[0].targets.Find(i => i.name == project).jsonFile;

        var target = LoadTargetModel(targetJson);

        return target.compileGroups[0].includes.Select(s=>s.path).ToList();
    }

    private static CoreModel? LoadCoreModel()
    {
        FileStream fs = new FileStream($"{cmakeFileAPI}codemodel-v2-36a0e6a8ef34aaa8865a.json", FileMode.Open);
        var a = JsonDocument.Parse(fs).Deserialize<CoreModel>();
        fs.Close();
        return a;
    }
    
    private static TargetModel? LoadTargetModel(string path)
    {
        FileStream fs = new FileStream($"{cmakeFileAPI}{path}", FileMode.Open);
        var a = JsonDocument.Parse(fs).Deserialize<TargetModel>();
        fs.Close();
        return a;
    }
}