﻿using System.CommandLine;
using System.CommandLine.Hosting;
using System.CommandLine.Invocation;
using System.CommandLine.NamingConventionBinder;
using System.CommandLine.Parsing;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.OutputWriter;

namespace shadow_header_tool;

public class GenerateCommand : Command
{
    //private static Option<string> _project = new Option<string>(name: "project", aliases: new []{"--project", "-p" });
    private static Option<string> _project = new (aliases: new []{"--project", "-p" }, description: "Name of the project to generate shadow headers for");
    //private static Option<string> _output = new Option<string>(name: "output", aliases: new []{"--output", "-o"});
    private static Option<string> _output = new (aliases: new []{"--output", "-o"}, description:"Output file for the generated shadow headers");
    
    private static Option<string> _cmake = new (aliases: new []{"--cmake-folder"}, description:"The cmake build directory");

    public GenerateCommand() : base("generate", "Generate shadow headers")
    {
        
        Add(_project);
        Add(_output);
        Add(_cmake);

        //this.Handler = CommandHandler.Create<Handler>();
    }
    
    public new class Handler : ICommandHandler
    {
        private readonly ICodeLoader _loader;
        private readonly IParser _codeParser;
        private readonly ICppReflectionDataWriter _dataWriter;

        public Handler(ICodeLoader loader, IParser codeParser, ICppReflectionDataWriter dataWriter)
        {
            _loader = loader;
            _codeParser = codeParser;
            _dataWriter = dataWriter;
        }

        public int Invoke(InvocationContext parseResult)
        {
            var project = parseResult.ParseResult.GetValueForOption(_project);
            if(project == null)
            {
                Console.WriteLine("No project specified");
                return 1;
            }
            var output = parseResult.ParseResult.GetValueForOption(_output);
            if(output == null)
            {
                Console.WriteLine("No output specified");
                return 1;
            }
            var cmake = parseResult.ParseResult.GetValueForOption(_cmake);
            if(cmake == null)
            {
                Console.WriteLine("No cmake folder specified");
                return 1;
            }
            
            Console.WriteLine(project);
            
            var exclude = new List<string>();
            exclude.Add(output);
            var files = _loader.GatherSourceFiles(cmake,project,exclude);
            
            _codeParser.AddSourceFiles(files, _loader.getIncludeDirs(project));
            var data = _codeParser.Process();
            
            _dataWriter.Write(output, data);
            return 0;
        }

        public Task<int> InvokeAsync(InvocationContext parseResult)
        {
            return Task.FromResult(Invoke(parseResult));
        }
    }
}