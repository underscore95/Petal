using Entry;
using Petal;

class Program
{
    public static void Main()
    {
        Engine engine = new();
        engine.SceneManager.SwitchScene(new TestScene());

        while (true)
        {
            engine.Update();
        }

        engine.Destroy();
    }
}