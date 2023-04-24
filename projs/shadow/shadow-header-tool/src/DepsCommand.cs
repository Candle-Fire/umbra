using System.CommandLine;
using System.CommandLine.Invocation;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.OutputWriter;

namespace shadow_header_tool;

public class DepsCommand : Command
{
    private static Option<string> _project = new (aliases: new []{"--project", "-p" }, description: "Name of the project to generate shadow headers for");
    
    public DepsCommand() : base("dependencies", "Prints out all the dependencies of a project")
    {
        Add(_project);
    }
    
    public new class Handler : ICommandHandler
    {
        private readonly ICodeLoader _loader;
        private readonly IParser _codeParser;

        public Handler(ICodeLoader loader, IParser codeParser, ICppReflectionDataWriter dataWriter)
        {
            _loader = loader;
            _codeParser = codeParser;
        }

        public int Invoke(InvocationContext parseResult)
        {
            var project = parseResult.ParseResult.GetValueForOption(_project);

            var exclude = new List<string>();
            var files = _loader.GatherSourceFiles(project,exclude);
            
            _codeParser.AddSourceFiles(files, _loader.getIncludeDirs(project));

            foreach (var file in _codeParser.GetFiles())
            {
                Console.WriteLine(file);
            }
            
            return 0;
        }

        public Task<int> InvokeAsync(InvocationContext parseResult)
        {
            return Task.FromResult(Invoke(parseResult));
        }
    }
}