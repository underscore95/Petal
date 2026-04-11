namespace Petal.Scenes;

public class SceneManager()
{
    private Scene? _scene;

    public void SwitchScene(Scene scene)
    {
        _scene = scene;
        _scene.Init();
    }

    public void Update()
    {
        _scene?.OnUpdate();
    }

    public void Destroy()
    {
        _scene?.OnDestroy();
    }
}