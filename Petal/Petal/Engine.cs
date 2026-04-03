using System.Diagnostics;
using Petal.Scenes;
using Petal.Utils;

namespace Petal;

public class Engine
{
    private static readonly Lazy<Engine> Instance = new(() => new Engine());

    public static Engine Singleton()
    {
        return Instance.Value;
    }
    
    public readonly SceneManager SceneManager;
    public bool IsDestroyed { get; private set; } = false;

    private Engine()
    {
        SceneManager = new SceneManager();
    }

    public void Update()
    {
        SceneManager.Update();
    }

    public void Destroy()
    {
        Debug.Assert(!IsDestroyed);
        
        SceneManager.Destroy();

        IsDestroyed = true;
    }
}