using System.CommandLine;
using System.CommandLine.Builder;
using System.CommandLine.Parsing;
using System.CommandLine.Hosting;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Serilog;
using shadow_header_tool;
using shadow_header_tool.CmakeLib;
using shadow_header_tool.CppSimpleParser;
using shadow_header_tool.FileCaching;
using shadow_header_tool.OutputWriter;
using shadow_header_tool.Services.CppAdvancedParser;

class Program
{
    private const string Version = "0.1.3";

    private const string Banner = @"
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
        
        var rootCommand = new RootCommand("Sample app for System.CommandLine");
        var noBanner = new Option<bool>( new []{"--no-banner", "-nb"}, "");
        rootCommand.AddGlobalOption( noBanner);
        rootCommand.Add(new GenerateCommand());
        rootCommand.Add(new DepsCommand());
        rootCommand.Add(new GenerateV2Command());
        
        
        
        var builder = new CommandLineBuilder(rootCommand);

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
                Console.WriteLine(Banner, Version);
                Console.WriteLine(string.Join(",", args));
                
            })
            .UseHost(_ => Host.CreateDefaultBuilder(args), configure => configure
                .ConfigureServices((_, services) =>
                {
                    services.AddSingleton(fileCache);
                    services.AddTransient<ICodeLoader, CmakeLoader>();
                    services.AddTransient<ICodeProcessor, AdvancedCodeProcessor>();
                    services.AddTransient<ICppReflectionDataWriter, CppReflectionDataWriter>();
                    services.AddSerilog();
                })
                .UseSerilog((_, _, config) =>
                {
                    config.MinimumLevel.Debug();
                    config.WriteTo.Console();
                })
                .ConfigureLogging((_, loggingBuilder) =>
                {
                    //loggingBuilder.ClearProviders();
                    loggingBuilder.AddSerilog(dispose: true);
                })
                //.UseCommandHandler<GenerateCommand, GenerateCommand.Handler>()
                .UseCommandHandler<GenerateV2Command, GenerateV2Command.Handler>()
                .UseCommandHandler<DepsCommand, DepsCommand.Handler>()
            )
            .Build();
        
        return await app.InvokeAsync(args);
    }
}