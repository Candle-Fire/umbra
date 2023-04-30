using System.Text.RegularExpressions;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.ReflectionModel;

namespace shadow_header_tool.Services.CppSimpleParser;

public class FileInfoEqualityComparer : IEqualityComparer<FileInclude>
{
    public bool Equals(FileInclude? x, FileInclude? y)
    {
        return x?.File.FullName == y?.File.FullName;
    }

    public int GetHashCode(FileInclude obj)
    {
        return obj.File.FullName.GetHashCode();
    }
}

public class FileInclude
{
    public FileInfo File;
    public string Include;
    
    public FileInclude(FileInfo file, string path)
    {
        File = file;
        Include = path;
    }
}

public class SimpleParser : IParser
{
    string classPattern = @"class (\[\[((?<attr>[\w:\(\)]+)(,\s)?)+\]\] )?(?<name>\w+) (:\s(?<parent>(public|private)? [\w:]+ )+)?\{(?<body>(?>\{(?<c>)|[^{}]+|\}(?<-c>))*(?(c)(?!)))\}";
    
    string includePattern = @"#include [<\""](?<path>[\w\/\\\.]+)[>\""]";
    
    string fieldPattern = @"(\[\[(?<attr>[\w:\(\)]+)?\]\] )?(?<type>\w+) (?<name>\w+)(\s?=[\s\w]+)?;";

    private string namespacePattern = @"namespace (?<ns>[\w\:]+) \{";

    private FileCache _fileCache;
    
    private HashSet<FileInclude> _files = new(new FileInfoEqualityComparer());

    public SimpleParser(FileCache cache)
    {
        _fileCache = cache;
    }
    
    public void AddSourceFiles(List<string> paths, List<string> includes)
    {
        foreach (var path in paths)
        {
            parse(path, includes);
        }
    }
    
    public List<FileInfo> GetFiles()
    {
        return _files.Select(f => f.File).ToList();
    }
    
    public void parse(string path, List<string> includes)
    {
        if (!path.EndsWith(".cpp") && !path.EndsWith(".h"))
        {
            return;
        }

        //Console.WriteLine($"########################################");
        //Console.WriteLine($"Parsing: {path}");
        
        var inc = includes.Select(i => new DirectoryInfo(i));

        var fileInfo = new FileInfo(path);
        if(!fileInfo.Exists)
        {
            return;
        }
        
        var file = _fileCache.ReadFile(fileInfo);
        
        foreach (Match m in Regex.Matches(file.content, includePattern, RegexOptions.Multiline))
        {
            
            var incFile = inc
                .Select(i => new FileInclude(
                    file: new FileInfo(i.FullName + "/" + m.Groups["path"].Value),
                    path: m.Groups["path"].Value
                ))
                .ToList()
                .Find(f => f.File.Exists);
            if (incFile != null)
            {
                //Console.WriteLine($"{m.Value, -60} is at: {incFile.File.FullName, 10}");
                _files.Add(incFile);
            }
            else
            {
                //Console.WriteLine($"{m.Value, -60} is not found");
            }
        }
    }

    public List<Clazz> Process()
    {
        List<Clazz> classes = new();

        foreach (var file in _files)
        {
            var fileData = _fileCache.ReadFile(file.File);
            
            var ns = Regex.Match(fileData.content, namespacePattern, RegexOptions.Multiline).Groups["ns"].Value;
            
            foreach (Match m in Regex.Matches(fileData.content, classPattern, RegexOptions.Multiline))
            {
                if (m.Groups["attr"].Captures.Select(i=>i.Value).Contains("SH::Reflect"))
                {
                    var name = ns +"::"+ m.Groups["name"].Value;
                    var clazz = new Clazz(name, file.Include);

                    var body = m.Groups["body"].Value;
                    foreach (Match f in Regex.Matches(body, fieldPattern, RegexOptions.Multiline))
                    {
                        var field = new Field(f.Groups["name"].Value, f.Groups["type"].Value);
                        clazz.Fields.Add(field);
                    }
                    
                    classes.Add(clazz);
                }
                
            }
        }

        return classes;
    }
}