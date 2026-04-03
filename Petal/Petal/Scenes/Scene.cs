namespace Petal.Scenes;

public abstract class Scene
{
    private bool _hasInitialized = false;
    protected readonly Engine Engine = Engine.Singleton();

    public void Init()
    {
        if (_hasInitialized) return;
        _hasInitialized = true;
        
        OnInit();
    }

    protected abstract void OnInit();
    public abstract void OnUpdate();
    public abstract void OnDestroy();
}