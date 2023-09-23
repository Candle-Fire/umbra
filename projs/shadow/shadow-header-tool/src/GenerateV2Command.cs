using System.CommandLine;
using System.CommandLine.Hosting;
using System.CommandLine.Invocation;
using System.CommandLine.NamingConventionBinder;
using System.CommandLine.Parsing;
using Microsoft.Extensions.Logging;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.OutputWriter;

namespace shadow_header_tool;

public class GenerateV2Command : Command
{
    private static Option<string> _project = new (aliases: new []{"--project", "-p" }, description: "Name of the project to generate shadow headers for");
    private static Option<string> _output = new (aliases: new []{"--output", "-o"}, description:"Output file for the generated shadow headers");
    
    
    private static Option<FileInfo[]> _files = new (aliases: new []{"--file", "-f"}, description:"The cpp of h file to process");
    private static Option<DirectoryInfo[]> _includes = new (aliases: new []{"--include", "-i"}, description:"The include folder");

    public GenerateV2Command() : base("generate_v2", "Generate shadow headers")
    {
        Add(_project);
        Add(_output);
        Add(_files);
        Add(_includes);
    }
    
    public new class Handler : ICommandHandler
    {
        private readonly Serilog.ILogger _logger;
        private readonly ICodeProcessor _codeCodeProcessor;
        private readonly ICppReflectionDataWriter _dataWriter;

        public Handler(Serilog.ILogger logger, ICodeProcessor codeCodeProcessor, ICppReflectionDataWriter dataWriter)
        {
            _logger = logger;
            _codeCodeProcessor = codeCodeProcessor;
            _dataWriter = dataWriter;
        }

        public int Invoke(InvocationContext parseResult)
        {
            var project = parseResult.ParseResult.GetValueForOption(_project);
            if(project == null)
            {
                _logger.Error("No project specified");
                return 1;
            }
            
            var output = parseResult.ParseResult.GetValueForOption(_output);
            if(output == null)
            {
                _logger.Error("No output specified");
                return 1;
            }
            
            var files = parseResult.ParseResult.GetValueForOption(_files);
            if(files is not null && files.Length  <= 0)
            {
                _logger.Error("No input specified");
                return 1;
            }
            
            var includes = parseResult.ParseResult.GetValueForOption(_includes);
            
            _logger.Information("Processing for project: {0}",project);

            var exclude = new List<string>(new []{output});
            try
            {
                _codeCodeProcessor.AddSourceFiles(files.Select(i=>i.FullName).ToList(), includes.Select(i=>i.FullName).ToList());
                var data = _codeCodeProcessor.Process();
            
                _dataWriter.Write(output, data);
            }
            catch (Exception e)
            {
                _logger.Fatal(e, "Exception while processing files");
                return 1;
            }
            return 0;
        }

        public Task<int> InvokeAsync(InvocationContext parseResult)
        {
            return Task.FromResult(Invoke(parseResult));
        }
    }
}