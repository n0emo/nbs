#include <raylib.h>

int main() {
    InitWindow(800, 600, "Hello VCPKG");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        EndDrawing();
    }
    return 0;
}
