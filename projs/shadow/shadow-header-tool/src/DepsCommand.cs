using System.CommandLine;
using System.CommandLine.Invocation;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.OutputWriter;

namespace shadow_header_tool;

public class DepsCommand : Command
{
    private static Option<string> _project = new (aliases: new []{"--project", "-p" }, description: "Name of the project to generate shadow headers for");
    
    private static Option<string> _cmake = new (aliases: new []{"--cmake-folder"}, description:"The cmake build directory");
    
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
            if(project == null)
            {
                Console.WriteLine("No project specified");
                return 1;
            }
            var cmake = parseResult.ParseResult.GetValueForOption(_cmake);
            if(cmake == null)
            {
                Console.WriteLine("No cmake folder specified");
                return 1;
            }
            
            var exclude = new List<string>();
            var files = _loader.GatherSourceFiles(cmake,project,exclude);
            
            _codeParser.AddSourceFiles(files, _loader.getIncludeDirs(project));

            Console.WriteLine(string.Join(";",_codeParser.GetFiles()));
            
            foreach (var file in _codeParser.GetFiles())
            {
                
            }
            
            return 0;
        }

        public Task<int> InvokeAsync(InvocationContext parseResult)
        {
            return Task.FromResult(Invoke(parseResult));
        }
    }
}