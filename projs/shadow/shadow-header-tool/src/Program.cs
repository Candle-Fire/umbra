using System.CommandLine;
using System.CommandLine.Builder;
using System.CommandLine.Invocation;
using System.CommandLine.Parsing;
using System.CommandLine.Hosting;
using System.CommandLine.NamingConventionBinder;
using CppAst;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using shadow_header_tool;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.OutputWriter;

class Program
{
    static string banner = @"
  (\
  .'.
  | |
  | |
  |_|-------------------------
    \_   Shadow Header Tool  |
      \_       v0.1.1        |
        \____________________|
    ";
    
    
    static async Task<int> Main(string[] args)
    {

        FileCache fileCache = new FileCache();
        
        var _rootCommand = new RootCommand("Sample app for System.CommandLine");
        var noBanner = new Option<bool>( new []{"--no-banner", "-nb"}, "");
        _rootCommand.AddGlobalOption( noBanner);
        _rootCommand.Add(new GenerateCommand());
        _rootCommand.Add(new DepsCommand());
        
        
        
        var builder = new CommandLineBuilder(_rootCommand);

        var app = builder
            .UseHelp()
            .UseVersionOption()
            .AddMiddleware(context =>
            {
                var nb = context.ParseResult.GetValueForOption(noBanner);
                if (nb)
                {
                    return;
                }
                Console.WriteLine(banner);
                Console.WriteLine(string.Join(",", args));
                
            })
            .UseHost(_ => Host.CreateDefaultBuilder(args), builder => builder
                .ConfigureServices((context, services) =>
                {
                    services.AddSingleton(fileCache);
                    services.AddTransient<ICodeLoader, CmakeLoader>();
                    services.AddTransient<IParser, SimpleParser>();
                    services.AddTransient<ICppReflectionDataWriter, CppReflectionDataWriter>();
                })
                .ConfigureLogging(loggingBuilder =>
                {
                    loggingBuilder.ClearProviders();
                })
                .UseCommandHandler<GenerateCommand, GenerateCommand.Handler>()
                .UseCommandHandler<DepsCommand, DepsCommand.Handler>()
            ).Build();


        /*
        CliConfiguration cliConfiguration = new CliConfiguration(rootCommand)
            .UseHost(
            _ => Host.CreateDefaultBuilder(args),
            builder =>
            {
                builder.ConfigureHostConfiguration(config =>
                {
                    config.AddCommandLine(args);
                });
                
                builder.ConfigureServices((context, services) =>
                {
                    services.AddSingleton(fileCache);
                    services.AddSingleton<ICodeLoader, CmakeLoader>();
                    //services.AddSingleton<ICppParser, CppParser>();
                    //services.AddSingleton<ICppReflectionDataWriter, CppReflectionDataWriter>();
                });

                builder.ConfigureLogging((host, logger) =>
                {
                    
                });
            });
        */
        return await app.InvokeAsync(args);
    }

    /*
                var loader = new CmakeLoader(fileCache);
                var exclude = new List<string>();
                exclude.Add(output);
                var files = loader.GatherSourceFiles(project,exclude);

                var parser = new shadow_header_tool.CppSimpleParser.SimpleParser(fileCache);
                parser.AddSourceFiles(files, loader.getIncludeDirs(project));
                var data = parser.Process();

                var writer = new CppReflectionDataWriter();
                writer.Write(output, data);
                */
}