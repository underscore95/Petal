using Entry;
using Petal;

class Program
{
    public static void Main()
    {
        Engine engine = Engine.Singleton();
        engine.SceneManager.SwitchScene(new TestScene());

        while (!engine.IsDestroyed)
        {
            engine.Update();
        }
    }
}