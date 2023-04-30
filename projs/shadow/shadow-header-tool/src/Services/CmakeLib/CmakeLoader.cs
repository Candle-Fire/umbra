using System.Text.Json;
using Microsoft.Extensions.Logging;
using shadow_header_tool.AssertHelpers;
using shadow_header_tool.CmakeLib.model;
using shadow_header_tool.FileCaching;
using shadow_header_tool.Services.CmakeLib.model;

namespace shadow_header_tool.CmakeLib;

public class CmakeLoader : ICodeLoader
{
    private readonly ILogger _logger;
    private readonly FileCache _cache;
    public string cmakeRoot = "";
    
    public CmakeLoader(ILogger logger,FileCache cache)
    {
        _logger = logger;
        _cache = cache;
    }

    public string CmakeFileApi
    {
        get => $"{cmakeRoot}/.cmake/api/v1/reply";
    }
    
    private TargetModel GetTarget(string project)
    {
        var a = LoadCoreModel();
        var targetJson = a.configurations[0].targets.Find(i => i.name == project)?.jsonFile;
        Asserts.Assert(targetJson != null, $"Failed to find project:{project}");
        
        var target = LoadTargetModel(targetJson!);
        Asserts.Assert(target != null, $"Failed to load target:{targetJson}");
        
        return target!;
    }
    
    public List<string> GatherSourceFiles(string cmake, string project, List<string> exclude)
    {
        cmakeRoot = cmake;
        var target = GetTarget(project);
        
        return target!.sources.Select(s=>s.path).Where(i=>!exclude.Contains(i)).ToList();
    }

    public List<string> GetIncludeDirs(string project)
    {
        var target = GetTarget(project);

        return target.compileGroups[0].includes.Select(s=>s.path).ToList();
    }

    private CodeModel LoadCoreModel()
    {
        DirectoryInfo api = new DirectoryInfo(CmakeFileApi);
        Asserts.Assert(api.Exists, $"Cmake api directory not found at:{api.FullName}");

        var codemodelFile = api.EnumerateFiles().ToList().Find(f => f.Name.StartsWith("codemodel-v2"));
        Asserts.Assert(codemodelFile == null || !codemodelFile.Exists, $"Cmake codemodel file not found at:{codemodelFile.FullName}");

        FileStream fs = new FileStream(codemodelFile.FullName, FileMode.Open);
        var a = JsonDocument.Parse(fs).Deserialize<CodeModel>();
        fs.Close();

        Asserts.Assert(a == null, $"Failed to deserialize codemodel file:{codemodelFile.FullName}");

        return a!;
    }
    
    private TargetModel? LoadTargetModel(string path)
    {
        FileInfo targetFile = new FileInfo($"{CmakeFileApi}/{path}");
        Asserts.Assert(targetFile.Exists, $"Cmake target file not found at:{targetFile.FullName}");

        var file = _cache.ReadFile(targetFile.FullName);
        var a = JsonDocument.Parse(file.content).Deserialize<TargetModel>();
        return a;
    }
}