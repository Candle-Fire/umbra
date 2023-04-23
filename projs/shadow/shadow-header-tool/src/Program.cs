using System.CommandLine;
using shadow_header_tool.CmakeLib;

class Program
{
    static async Task<int> Main(string[] args)
    {
        Console.WriteLine("asdasd");

        var fileOption = new Option<string>(
            name: "--proj",
            description: "The project to generate for");

        var rootCommand = new RootCommand("Sample app for System.CommandLine");
        rootCommand.AddOption(fileOption);

        rootCommand.SetHandler((file) =>
            {
                Console.WriteLine(file);
                var loader = new CmakeLoader();
                var files = loader.getFiles(file);

                var arser = new shadow_header_tool.CppProcessor.Parser();
                files.ForEach(s=>
                {
                    Console.WriteLine(s);
                    arser.parse(s, loader.getIncludeDirs(file));
                });
                
            },
            fileOption);

        return await rootCommand.InvokeAsync(args);
    }
}