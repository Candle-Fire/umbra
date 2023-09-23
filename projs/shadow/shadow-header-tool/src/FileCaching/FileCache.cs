namespace shadow_header_tool.FileCaching;

public class FileCache
{
    private Dictionary<FileInfo, VirtualFile> files = new();

    public class VirtualFile
    {
        public string content;
        public FileInfo file;

        public VirtualFile(string path)
        {
            file = new FileInfo(path);
            
            var s = new StreamReader(file.FullName);
            content = s.ReadToEnd();
            s.Close();
        }
    }

    public VirtualFile ReadFile(string path)
    {
        var fileInfo = new FileInfo(path);
        return ReadFile(fileInfo);
    }
    
    public VirtualFile ReadFile(FileInfo file)
    {
        if (!files.ContainsKey(file))
        {
            files.Add(file, new VirtualFile(file.FullName));
        }
        
        return files[file];
    }
}