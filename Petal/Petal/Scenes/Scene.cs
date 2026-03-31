namespace Petal.Scenes;

public abstract class Scene
{
    private bool _hasInitialized = false;
    protected Engine? Engine;

    public void Init(Engine engine)
    {
        if (_hasInitialized) return;
        _hasInitialized = true;
        Engine = engine;
        
        OnInit();
    }

    protected abstract void OnInit();
    public abstract void OnUpdate();
    public abstract void OnDestroy();
}