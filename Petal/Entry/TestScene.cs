using System.Drawing;
using System.Numerics;
using Petal.Scenes;
using Petal.Window;

namespace Entry;

public class TestScene : Scene
{
    protected override void OnInit()
    {
        Console.WriteLine("Init");
    }

    public override void OnUpdate()
    {
    }

    public override void OnDestroy()
    {
        Console.WriteLine("Destroy");
    }
}