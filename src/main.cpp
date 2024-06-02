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
    // 'X', 'O', or '-'
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
    // array for storing the board
    std::vector<std::vector<char>> board;
    std::vector<BoardPosition> winningPositions;
};

class GameSystem : public System
{
private:
    bool RowWinner(GameStatus &gameStatus, int row)
    {
        return gameStatus.board[row][0] == gameStatus.board[row][1] && gameStatus.board[row][1] == gameStatus.board[row][2] && gameStatus.board[row][0] != '-';
    }

    bool ColumnWinner(GameStatus &gameStatus, int col)
    {
        return gameStatus.board[0][col] == gameStatus.board[1][col] && gameStatus.board[1][col] == gameStatus.board[2][col] && gameStatus.board[0][col] != '-';
    }

    bool ForwardDiagonalWinner(GameStatus &gameStatus)
    {
        return gameStatus.board[0][0] == gameStatus.board[1][1] && gameStatus.board[1][1] == gameStatus.board[2][2] && gameStatus.board[0][0] != '-';
    }

    bool BackwardDiagonalWinner(GameStatus &gameStatus)
    {
        return gameStatus.board[0][2] == gameStatus.board[1][1] && gameStatus.board[1][1] == gameStatus.board[2][0] && gameStatus.board[0][2] != '-';
    }

    bool Draw(GameStatus &gameStatus)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (gameStatus.board[i][j] == '-')
                {
                    return false;
                }
            }
        }

        return true;
    }

public:
    void Update(Entity game)
    {
        auto &gameStatus = gCoordinator.GetComponent<GameStatus>(game);
        auto &playerTurn = gCoordinator.GetComponent<PlayerTurn>(game);

        // Check for winner
        for (int i = 0; i < 3; i++)
        {
            // @TODO: combine row and column checks.
            if (RowWinner(gameStatus, i))
            {
                gameStatus.status = gameStatus.board[i][0] == 'X' ? GameStatusEnum::X_WIN : GameStatusEnum::O_WIN;
                gameStatus.winningPositions = {BoardPosition{i, 0}, BoardPosition{i, 1}, BoardPosition{i, 2}};
            }
            else if (ColumnWinner(gameStatus, i))
            {
                gameStatus.status = gameStatus.board[0][i] == 'X' ? GameStatusEnum::X_WIN : GameStatusEnum::O_WIN;
                gameStatus.winningPositions = {BoardPosition{0, i}, BoardPosition{1, i}, BoardPosition{2, i}};
            }
            else if (ForwardDiagonalWinner(gameStatus))
            {
                gameStatus.status = gameStatus.board[0][0] == 'X' ? GameStatusEnum::X_WIN : GameStatusEnum::O_WIN;
                gameStatus.winningPositions = {BoardPosition{0, 0}, BoardPosition{1, 1}, BoardPosition{2, 2}};
            }
            else if (BackwardDiagonalWinner(gameStatus))
            {
                gameStatus.status = gameStatus.board[0][2] == 'X' ? GameStatusEnum::X_WIN : GameStatusEnum::O_WIN;
                gameStatus.winningPositions = {BoardPosition{0, 2}, BoardPosition{1, 1}, BoardPosition{2, 0}};
            }
            else if (Draw(gameStatus))
            {
                gameStatus.status = GameStatusEnum::DRAW;
            }
        }
    }
};

class InputSystem : public System
{
private:
    void UpdateGameBoard(Entity game, char symbol, BoardPosition move)
    {
        auto &gameStatus = gCoordinator.GetComponent<GameStatus>(game);
        gameStatus.board[move.row][move.col] = symbol;
    }

    void CheckCellCollision(Entity game)
    {
        auto mousePosition = GetMousePosition();
        auto gameStatus = gCoordinator.GetComponent<GameStatus>(game);
        auto &playerTurn = gCoordinator.GetComponent<PlayerTurn>(game);

        for (auto const &entity : mEntities)
        {
            auto &cell = gCoordinator.GetComponent<GridCell>(entity);
            auto &boardPosition = gCoordinator.GetComponent<BoardPosition>(entity);

            if (CheckCollisionPointRec(mousePosition, cell.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (cell.value == '-')
                {
                    cell.value = playerTurn.symbol;
                    UpdateGameBoard(game, playerTurn.symbol, boardPosition);

                    playerTurn.symbol = playerTurn.symbol == 'X' ? 'O' : 'X';
                }
            }
        }
    }

    void CheckResetButtonCollision(Entity game)
    {
        auto mousePosition = GetMousePosition();
        auto &resetButton = gCoordinator.GetComponent<ResetButton>(game);

        if (CheckCollisionPointRec(mousePosition, resetButton.rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            auto &gameStatus = gCoordinator.GetComponent<GameStatus>(game);
            gameStatus = GameStatus{GameStatusEnum::PLAYING, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}};

            for (auto const &entity : mEntities)
            {
                auto &cell = gCoordinator.GetComponent<GridCell>(entity);
                cell.value = '-';
            }
        }
    }

public:
    void Update(Entity game)
    {
        auto &gameStatus = gCoordinator.GetComponent<GameStatus>(game);

        if (gameStatus.status == GameStatusEnum::PLAYING)
        {
            CheckCellCollision(game);
        }

        CheckResetButtonCollision(game);
    }
};

// Systems
class RenderSystem : public System
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
        BeginDrawing();
        ClearBackground(RAYWHITE);

        RenderResetButton(game);

        for (auto const &entity : mEntities)
        {
            auto cell = gCoordinator.GetComponent<GridCell>(entity);
            auto boardPosition = gCoordinator.GetComponent<BoardPosition>(entity);

            auto gameStatus = gCoordinator.GetComponent<GameStatus>(game);

            bool isWinningPosition = false;
            for (auto winningPosition : gameStatus.winningPositions)
            {
                if (winningPosition.row == boardPosition.row && winningPosition.col == boardPosition.col)
                {
                    isWinningPosition = true;
                    break;
                }
            }

            DrawRectangleRec(cell.rect, isWinningPosition ? GREEN : LIGHTGRAY);
            DrawRectangleLines(cell.rect.x, cell.rect.y, cell.rect.width, cell.rect.height, BLACK);
            // @FIXME: this is drawing weird characters
            // DrawText(&cell.value, cell.rect.x + 50, cell.rect.y + 50, 50, BLACK);
            if (cell.value == 'X')
            {
                DrawText("X", cell.rect.x + 50, cell.rect.y + 50, 50, BLACK);
            }
            else if (cell.value == 'O')
            {
                DrawText("O", cell.rect.x + 50, cell.rect.y + 50, 50, BLACK);
            }
            else if (cell.value == '-')
            {
                DrawText("-", cell.rect.x + 50, cell.rect.y + 50, 50, BLACK);
            }
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
    // gCoordinator.AddComponent(game, GameStatus{GameStatusEnum::PLAYING, {}});
    gCoordinator.AddComponent(game, GameStatus{GameStatusEnum::PLAYING, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}});
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

    auto renderSystem = gCoordinator.RegisterSystem<RenderSystem>();
    auto inputSystem = gCoordinator.RegisterSystem<InputSystem>();
    auto gameSystem = gCoordinator.RegisterSystem<GameSystem>();

    Signature renderSystemSignature;
    renderSystemSignature.set(gCoordinator.GetComponentType<GridCell>());
    gCoordinator.SetSystemSignature<RenderSystem>(renderSystemSignature);
    gCoordinator.SetSystemSignature<InputSystem>(renderSystemSignature);

    auto game = CreateGame();
    CreateCells();

    while (!WindowShouldClose())
    {
        inputSystem->Update(game);
        gameSystem->Update(game);
        renderSystem->Update(game);
    }

    CloseWindow();
}
