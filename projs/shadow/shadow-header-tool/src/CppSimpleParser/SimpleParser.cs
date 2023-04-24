using System.Text.RegularExpressions;
using shadow_header_tool.FileCaching;
using shadow_header_tool.ReflectionModel;

namespace shadow_header_tool.CppSimpleParser;

public class FileInfoEqualityComparer : IEqualityComparer<FileInclude>
{
    public bool Equals(FileInclude x, FileInclude y)
    {
        return x.File.FullName == y.File.FullName;
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

public interface IParser
{
    void AddSourceFiles(List<string> paths, List<string> includes);
    List<Clazz> Process();
    List<FileInfo> GetFiles();
}

public class SimpleParser : IParser
{
    string classPattern = @"class (\[\[(?<attr>[\w:\(\)]+)?\]\] )?(?<name>\w+) (:\s(?<parent>(public|private)? [\w:]+ )+)?\{(?<body>(?>\{(?<c>)|[^{}]+|\}(?<-c>))*(?(c)(?!)))\}";
    
    string includePattern = @"#include [<\""](?<path>[\w\/\\\.]+)[>\""]";
    
    string fieldPattern = @"(\[\[(?<attr>[\w:\(\)]+)?\]\] )?(?<type>\w+) (?<name>\w+)(\s?=[\s\w]+)?;";

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

        var file = _fileCache.ReadFile(path);
        
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
            
            foreach (Match m in Regex.Matches(fileData.content, classPattern, RegexOptions.Multiline))
            {
                if (m.Groups["attr"].Value == "SH::Reflect")
                {
                    var clazz = new Clazz();
                    clazz.Include = file.Include;
                    clazz.Name = m.Groups["name"].Value;
                    
                    var body = m.Groups["body"].Value;
                    foreach (Match f in Regex.Matches(body, fieldPattern, RegexOptions.Multiline))
                    {
                        var field = new Field();
                        field.Name = f.Groups["name"].Value;
                        field.Type = f.Groups["type"].Value;
                        clazz.Fields.Add(field);
                    }
                    
                    classes.Add(clazz);
                }
                
            }
        }

        return classes;
    }
}