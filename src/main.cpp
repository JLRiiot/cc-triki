#include "entity-component-system.h"
#include <raylib.h>
#include <vector>
#include <string>
#include <string>
#include <vector>

Coordinator gCoordinator;

// Components

struct BoardPosition
{
    int row;
    int col;
};

struct GridCell
{
    // 'X', 'O', or ' '
    char value;
    Rectangle rect;
};

struct PlayerTurn
{
    bool isActive;
};

struct GameStatus
{
    std::string status;
    std::vector<BoardPosition> winningPositions;
};

// Systems
class InputSystem
{
public:
    void Update()
    {
    }
};

class RenderSystem : public System
{
public:
    void Update()
    {

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Triki game made by cpo.dev!", 190, 200, 20, DARKGRAY);

        for (auto const &entity : mEntities)
        {
            auto &cell = gCoordinator.GetComponent<GridCell>(entity);
            DrawRectangleRec(cell.rect, GREEN);
            DrawRectangleLines(cell.rect.x, cell.rect.y, cell.rect.width, cell.rect.height, BLACK);
            DrawText(&cell.value, cell.rect.x + 50, cell.rect.y + 50, 50, BLACK);
        }

        EndDrawing();
    }
};

void CreateCells()
{
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            auto cell = gCoordinator.CreateEntity();
            gCoordinator.AddComponent(cell, BoardPosition{row, col});
            auto x = col * 200.0f;
            auto y = row * 200.0f;
            auto width = 200.0f;
            auto height = 200.0f;

            gCoordinator.AddComponent(cell, GridCell{'-', Rectangle{x, y, width, height}});
        }
    }
}

int main()
{
    InitWindow(800, 600, "Triki!");
    gCoordinator.Init();
    gCoordinator.RegisterComponent<BoardPosition>();
    gCoordinator.RegisterComponent<GridCell>();

    auto renderSystem = gCoordinator.RegisterSystem<RenderSystem>();
    Signature renderSystemSignature;
    renderSystemSignature.set(gCoordinator.GetComponentType<GridCell>());
    gCoordinator.SetSystemSignature<RenderSystem>(renderSystemSignature);

    CreateCells();

    while (!WindowShouldClose())
    {
        renderSystem->Update();
    }

    CloseWindow();
}
