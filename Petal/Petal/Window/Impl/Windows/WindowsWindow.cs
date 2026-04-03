using System.Numerics;

namespace Petal.Window.Impl.Windows;

public sealed class WindowsWindow(string initialTitle, Vector2 initialSize) : Window(initialTitle, initialSize)
{

    public override void Update()
    {
        throw new NotImplementedException();
    }

    public override void Render()
    {
        throw new NotImplementedException();
    }

    protected override void SetTitle(string title)
    {
        
    }

    protected override void SetSize(Vector2 size)
    {
    }
}