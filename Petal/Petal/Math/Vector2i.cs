using System.Numerics;

namespace Petal.Math;

public struct Vector2i
{
    public int X { get; set; }
    public int Y { get; set; }

    public Vector2i(int x, int y)
    {
        X = x;
        Y = y;
    }

    public Vector2i()
    {
        X = 0;
        Y = 0;
    }

    public static explicit operator Vector2i(Vector2 v)
    {
        return new Vector2i((int)v.X, (int)v.Y);
    }

    public static explicit operator Vector2(Vector2i v)
    {
        return new Vector2(v.X, v.Y);
    }
}