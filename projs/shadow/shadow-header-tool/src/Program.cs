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
using Serilog;
using shadow_header_tool;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.OutputWriter;
using shadow_header_tool.Services.CppAdvancedParser;
using shadow_header_tool.Services.CppSimpleParser;

class Program
{
    static string version = "0.1.2";
    static string banner = @"
  (\
  .'.
  | |
  | |
  |_|--------------------------|
    \_   Shadow Header Tool    |
      \_       v{0, -8}       |
        \______________________|
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
                Console.WriteLine(string.Format(banner,version));
                Console.WriteLine(string.Join(",", args));
                
            })
            .UseHost(_ => Host.CreateDefaultBuilder(args), configure => configure
                .ConfigureServices((context, services) =>
                {
                    services.AddSingleton(fileCache);
                    services.AddTransient<ICodeLoader, CmakeLoader>();
                    services.AddTransient<ICodeProcessor, AdvancedCodeProcessor>();
                    services.AddTransient<ICppReflectionDataWriter, CppReflectionDataWriter>();
                    services.AddSerilog();
                })
                .UseSerilog((context, provider, config) =>
                {
                    config.MinimumLevel.Debug();
                    config.WriteTo.Console();
                })
                .ConfigureLogging((context, loggingBuilder) =>
                {
                    //loggingBuilder.ClearProviders();
                    loggingBuilder.AddSerilog(dispose: true);
                })
                .UseCommandHandler<GenerateCommand, GenerateCommand.Handler>()
                .UseCommandHandler<DepsCommand, DepsCommand.Handler>()
            )
            .Build();
        
        return await app.InvokeAsync(args);
    }
}