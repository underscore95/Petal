using Petal.Scenes;

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