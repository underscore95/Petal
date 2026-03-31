using Petal.Scenes;

namespace Petal;

public class Engine
{
    public readonly SceneManager SceneManager;

    public Engine()
    {
        SceneManager = new SceneManager(this);
    }

    public void Update()
    {
        SceneManager.Update();
    }

    public void Destroy()
    {
        SceneManager.Destroy();
    }
}