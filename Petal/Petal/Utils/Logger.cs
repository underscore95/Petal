using System.Diagnostics.CodeAnalysis;

namespace Petal.Utils;

public class Logger(Logger.Level initialLevel)
{
    public enum Level
    {
        Info = 0,
        Warning = 1,
        Error = 2
    }

    public Level LoggerLevel { get; set; } = initialLevel;

    public void Log(Level level, [StringSyntax("CompositeFormat")] string format, params object[] args)
    {
        if (LoggerLevel > level) return;
        Console.WriteLine("[{0}] {1}", level, string.Format(format, args));
    }

    public void Info([StringSyntax("CompositeFormat")] string format, params object[] args)
    {
        Log(Level.Info, format, args);
    }

    public void Warning([StringSyntax("CompositeFormat")] string format, params object[] args)
    {
        Log(Level.Warning, format, args);
    }

    public void Error([StringSyntax("CompositeFormat")] string format, params object[] args)
    {
        Log(Level.Error, format, args);
    }
}