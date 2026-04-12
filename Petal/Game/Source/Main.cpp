#include "Petal.h"

int main() {
    using namespace Petal;

    Engine engine;
    Ref<Window> window = engine.GetWindowSystem().OpenWindow();
    while (true) {
        window->Update();
        window->Render();
    }
    engine.GetWindowSystem().CloseWindow(window);
    return 0;
}
