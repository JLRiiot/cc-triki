#include "triqui.h"
#include "entity-component-system.h"
#include <raylib.h>
#include <vector>
#include <string>

int main()
{
    triqui();
    InitWindow(800, 600, "Hello, World!");
    EntityManager manager{};
    auto player1 = manager.CreateEntity();
    auto player2 = manager.CreateEntity();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
}
