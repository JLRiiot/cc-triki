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
    char symbol;
};

struct ResetButton
{
    Rectangle rect;
};

// Enum for game status
enum class GameStatusEnum
{
    PLAYING,
    DRAW,
    X_WIN,
    O_WIN
};

struct GameStatus
{
    GameStatusEnum status;
    std::vector<BoardPosition> winningPositions;
};

// Systems
class GameSystem : public System
{
private:
    void RenderResetButton(Entity game)
    {
        auto &resetButton = gCoordinator.GetComponent<ResetButton>(game);
        DrawRectangleRec(resetButton.rect, RED);
        DrawText("Reset", resetButton.rect.x + 50, resetButton.rect.y + 50, 50, BLACK);
    }

public:
    void Update(Entity game)
    {
        auto mousePosition = GetMousePosition();
        auto &playerTurn = gCoordinator.GetComponent<PlayerTurn>(game);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        RenderResetButton(game);

        for (auto const &entity : mEntities)
        {
            auto &cell = gCoordinator.GetComponent<GridCell>(entity);

            if (CheckCollisionPointRec(mousePosition, cell.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (cell.value != ' ')
                {
                    cell.value = playerTurn.symbol;
                    playerTurn.symbol = playerTurn.symbol == 'X' ? 'O' : 'X';
                }
            }

            DrawRectangleRec(cell.rect, LIGHTGRAY);
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

Entity CreateGame()
{
    auto game = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(game, GameStatus{GameStatusEnum::PLAYING, {}});
    gCoordinator.AddComponent(game, PlayerTurn{'X'});
    gCoordinator.AddComponent(game, ResetButton{Rectangle{600, 400, 200, 100}});

    return game;
}

int main()
{
    InitWindow(800, 600, "Triki!");
    gCoordinator.Init();
    gCoordinator.RegisterComponent<BoardPosition>();
    gCoordinator.RegisterComponent<GridCell>();
    gCoordinator.RegisterComponent<GameStatus>();
    gCoordinator.RegisterComponent<PlayerTurn>();
    gCoordinator.RegisterComponent<ResetButton>();

    auto renderSystem = gCoordinator.RegisterSystem<GameSystem>();

    Signature renderSystemSignature;
    renderSystemSignature.set(gCoordinator.GetComponentType<GridCell>());
    gCoordinator.SetSystemSignature<GameSystem>(renderSystemSignature);

    auto game = CreateGame();
    CreateCells();

    while (!WindowShouldClose())
    {
        renderSystem->Update(game);
    }

    CloseWindow();
}
