using Petal.Math;

namespace Petal.Window;

public readonly record struct WindowResizeInfo(Vector2i PreviousSize, WindowResizeCause Cause);