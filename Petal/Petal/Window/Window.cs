using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using Petal.Math;
using Silk.NET.Input;
using Silk.NET.Windowing;

namespace Petal.Window;

public class Window
{
    private IWindow _window;
    private string _title;
    private Vector2i _size;
    private IInputContext _input;
    private readonly HashSet<Key> _pressedKeysList = new(); // List of all keys pressed this frame
    private readonly Dictionary<Key, bool> _keyStates = new(); // True if down, false if not down
    public bool IsOpen { get; private set; } = true;

    // Create and open the window
    public Window(string initialTitle = "Petal") : this(initialTitle, new Vector2i(1280, 720))
    {
    }

    public Window(string initialTitle, Vector2i initialSize)
    {
        _size = initialSize;
        _title = initialTitle;

        CreateWindow();
    }

    // Close the window
    public void Close()
    {
        if (IsOpen)
        {
            _window.Close();
            IsOpen = false;
        }
    }

    public void Update()
    {
        _pressedKeysList.Clear();
        _window.DoEvents();
        _window.DoUpdate();

        if (_window.IsClosing)
        {
            Close();
        }
    }

    public void Render()
    {
        _window.DoRender();
    }

    // Check if key was pressed this frame
    public bool IsKeyPressed(Key key)
    {
        return _pressedKeysList.Contains(key);
    }

    // Check if key is currently down
    public bool IsKeyDown(Key key)
    {
        return _keyStates.TryGetValue(key, out bool state) && state;
    }

    // Called after window resize
    public Action<WindowResizeInfo> OnResize { get; set; } = _ => { };

    public string Title
    {
        get => _title;
        set
        {
            _title = value;
            _window.Title = value;
        }
    }

    public Vector2i Size
    {
        get => _size;
        set
        {
            Vector2i previousSize = _size;
            _size = value;
            _window.Size = new(_size.X, _size.Y);
            OnResize.Invoke(new(previousSize, WindowResizeCause.Developer));
        }
    }

    private void HandleKeyDown(IKeyboard keyboard, Key key, int keyCode)
    {
        _keyStates[key] = true;
        _pressedKeysList.Add(key);
    }

    private void HandleKeyUp(IKeyboard keyboard, Key key, int keyCode)
    {
        _keyStates[key] = false;
    }

    [MemberNotNull(nameof(_window), nameof(_input))]
    private void CreateWindow()
    {
        var options = WindowOptions.DefaultVulkan with
        {
            Size = new(Size.X, Size.Y),
            Title = Title
        };

        _window = Silk.NET.Windowing.Window.Create(options);

        _window.FramebufferResize += size =>
        {
            Vector2i previousSize = _size;
            _size = new(size.X, size.Y);
            OnResize.Invoke(new(previousSize, WindowResizeCause.User));
        };

        _window.Initialize();

        _input = _window.CreateInput();
        foreach (var keyboard in _input.Keyboards)
        {
            keyboard.KeyDown += HandleKeyDown;
            keyboard.KeyUp += HandleKeyUp;
        }
    }
}