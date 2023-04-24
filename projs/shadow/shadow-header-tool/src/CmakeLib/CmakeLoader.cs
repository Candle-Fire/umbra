﻿using System.Text.Json;
using shadow_header_tool.CmakeLib.model;
using shadow_header_tool.FileCaching;

namespace shadow_header_tool.CmakeLib;

public class CmakeLoader : ICodeLoader
{
    private readonly FileCache _cache;
    public static string cmakeRoot = "./bin/debug/cmake-build/MinGW";
    public static string cmakeFileAPI = $"{cmakeRoot}/.cmake/api/v1/reply";


    public CmakeLoader(FileCache cache)
    {
        _cache = cache;
    }
    
    public List<string> GatherSourceFiles(string project, List<string> exclude)
    {
        var a = LoadCoreModel();
        var targetJson = a.configurations[0].targets.Find(i => i.name == project).jsonFile;

        var target = LoadTargetModel(targetJson);

        return target.sources.Select(s=>s.path).Where(i=>!exclude.Contains(i)).ToList();
    }

    public List<string> getIncludeDirs(string project)
    {
        var a = LoadCoreModel();
        var targetJson = a.configurations[0].targets.Find(i => i.name == project).jsonFile;

        var target = LoadTargetModel(targetJson);

        return target.compileGroups[0].includes.Select(s=>s.path).ToList();
    }

    private static CodeModel? LoadCoreModel()
    {
        DirectoryInfo api = new DirectoryInfo(cmakeFileAPI);
        var codemodelFile = api.EnumerateFiles().ToList().Find(f => f.Name.StartsWith("codemodel-v2"));
        FileStream fs = new FileStream(codemodelFile.FullName, FileMode.Open);
        var a = JsonDocument.Parse(fs).Deserialize<CodeModel>();
        fs.Close();
        return a;
    }
    
    private TargetModel? LoadTargetModel(string path)
    {
        var file = _cache.ReadFile($"{cmakeFileAPI}/{path}");
        var a = JsonDocument.Parse(file.content).Deserialize<TargetModel>();
        return a;
    }
}