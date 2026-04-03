using System.Drawing;
using System.Numerics;
using Petal.Scenes;
using Petal.Utils;
using Petal.Window;
using Silk.NET.Input;

namespace Entry;

public class TestScene : Scene
{
    private Window _window = new();

    protected override void OnInit()
    {
        Console.WriteLine("Init");
    }

    public override void OnUpdate()
    {
        _window.Update();

        if (_window.IsKeyPressed(Key.Escape))
        {
            _window.Close();
        }

        if (!_window.IsOpen)
        {
            Engine.Destroy();
        }
    }

    public override void OnDestroy()
    {
        Console.WriteLine("Destroy");
        _window.Close();
    }
}