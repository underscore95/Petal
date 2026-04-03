using System.Numerics;

namespace Petal.Window;

public abstract class Window(string initialTitle, Vector2 initialSize)
{
    private string _title = initialTitle;

    public string Title
    {
        get => _title;
        set
        {
            _title = value;
            SetTitle(value);
        }
    }

    private Vector2 _size = initialSize;

    public Vector2 Size
    {
        get => _size;
        set
        {
            _size = value;
            SetSize(value);
        }
    }

    public abstract void Update();
    public abstract void Render();

    protected abstract void SetTitle(string title);
    protected abstract void SetSize(Vector2 size);
}