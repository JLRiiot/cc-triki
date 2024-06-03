# Triki

Is how we call Tic Tac Toe at my country (Colombia), AKA Triqui and Tres en linea. It has a very simple logic and is the perfect challenge to start practicing basic concepts of any language.

In this case my implementation uses `C++`, `Raylib`, `ECS` and `conan`

## Features
The game board is divided into cells, each of which can be marked with `X`, `O`, or `-` (indicating an empty cell).
The game checks for a winning position after each move. If a winning position is found, it is highlighted in green.
The game supports a graphical user interface, with the game board displayed in a window. The state of each cell is rendered as text within the corresponding rectangle on the board.
Code Structure

The `main.cpp` file contains the main game loop, as well as functions for drawing the game board and handling user input.

The `DrawRectangleRec` function is used to draw the rectangles representing the cells on the game board. If a cell is part of a winning position, it is drawn in green; otherwise, it is drawn in light gray.

The `DrawRectangleLines` function is used to draw the lines separating the cells on the game board.

The `DrawText` function is used to draw the state of each cell (`X`, `O`, or `-`) as text within the corresponding rectangle on the game board.

## Requirements

- Raylib

## How to Run
To run the game, you need to compile the main.cpp file and link it with the `Raylib` library.

This repo provides a `conanfile.py` configured for development ONLY, please follow these steps to compile using `conan`.

```bash
# Install the dependencies
conan install . --build=missing -s build_type=Debug

# Configure preset
cmake --preset conan-debug

# Build using configured preset
cmake --build --preset conan-debug

# Run
./build/Debug/triqui
```

## Future Improvements
Currently, the game does not support player vs AI gameplay. This could be added in the future.

The game also does not currently support resizing or scaling of the game window. This could be improved to make the game more flexible and user-friendly.

## Contributions
Contributions to this project are welcome. If you have a feature you'd like to add, or a bug you'd like to fix, please open a pull request.