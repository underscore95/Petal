namespace Petal.Scenes;

public class SceneManager(Engine engine)
{
    private Scene? _scene;

    public void SwitchScene(Scene scene)
    {
        _scene = scene;
        _scene.Init(engine);
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