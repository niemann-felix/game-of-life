/*******************************************************
*
* Conway's Game of Life
*
* This is a reinterpretation of one of the very first
* projects I ever did in a low level language. The code
* quality and performance - as far as I remember -
* was kinda bad, although there are some aspects I 
* wanna improve in this implementation in the future too.
* 
* Some fun things you can do for performance:
*   - https://en.wikipedia.org/wiki/Hashlife
*   - https://en.wikipedia.org/wiki/Quadtree
*
* Copyright (c) 2024 Felix Niemann
*
********************************************************/

#include "raylib.h"
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "raymath.h"
#define ROWS 300 
#define COLS 300 

#define colorGridNonAlive BLACK
#define colorBackground GetColor(0x151515FF)
#define colorGridAlive GetColor (0xFF80edFF)
#define colorInfoPanel GetColor(0xc7d2d6FF)

typedef struct{
    int current_state;
    int next_state;
    int neighbors;
    int next_neighbors;
} Cell;

typedef struct {
    Cell *items;
    size_t size;
} CellRow;

typedef struct {
    CellRow *rows;
    size_t size;
} CellGrid;

void drawMainGrid(CellGrid *grid, Vector2 *gridPosition, float *zoomLevel){
    for (size_t i = 0; i<ROWS; i++){
        for (size_t j = 0; j<COLS; j++){
            Vector2 blockSize = {.x = 1*(*zoomLevel), .y=1*(*zoomLevel)};
            Vector2 position = {.y = i*(*zoomLevel), .x=j*(*zoomLevel)};
            if(position.x + gridPosition->x > GetScreenWidth() || position.y + gridPosition->y > GetScreenHeight() || position.y < 0 || position.x < 0){
                continue;
            }
            DrawRectangleV(Vector2Add(*gridPosition, position) , blockSize, (grid->rows[i].items[j].current_state & 1) ? colorGridAlive : colorGridNonAlive); 
        }
    }
}

void drawPlayingButton(Vector2 *position, bool *playing, Font *font){
    DrawRectangleV(Vector2Subtract(*position, (Vector2){.x=0,.y=5}), (Vector2){.x=55,.y=20}, BLACK);
    if(*playing){
        Vector2 v1 = Vector2Add(*position, (Vector2){.x=5, .y=0});
        Vector2 v2 = Vector2Add(*position, (Vector2){.x=5, .y=10});
        Vector2 v3 = Vector2Add(*position, (Vector2){.x=15, .y=5});
        DrawTriangle(v1, v2, v3, GREEN);
        const char *text = "Play";
        DrawTextEx(*font, text, Vector2Add(*position, (Vector2){.x=20, .y=0}), 12, 0.3, WHITE);
    }
    else {
        DrawRectangleV(Vector2Add(*position, (Vector2){.x=5, .y=0}), (Vector2){.x=10,.y=10}, RED);
        const char *text = "Stop";
        DrawTextEx(*font, text, Vector2Add(*position, (Vector2){.x=20, .y=0}), 12, 0.3, WHITE);
    }
}

void update_neighbors(CellGrid *grid, int row, int col, int amount){
    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            // Check if the neighboring cell is within the valid range
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                if(i == row && j == col){
                    ;
                }
                else{
                    grid->rows[i].items[j].next_neighbors += amount;
                }
            }
        }
    } 
}

void apply_next(CellGrid *grid){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t col = 0; col < COLS; col++){
            Cell *cell = &grid->rows[row].items[col];
            cell->current_state = cell->next_state;
            cell->neighbors = cell->next_neighbors;
        }
    }
}

void generate_next(CellGrid *grid){
    for (size_t row = 0; row < ROWS; row++){
        for (size_t col = 0; col < COLS; col++){

            Cell *cell = &grid->rows[row].items[col]; // get a pointer to the cell that were working with rn

            if(cell->current_state == 0){
                if(cell->neighbors == 3){
                    cell->next_state = 1;
                    update_neighbors(grid, row, col, 1);
                }
            }
            else{ //current state == 1 
                if(cell->neighbors <= 1){
                    cell->next_state = 0;
                    update_neighbors(grid, row, col, -1);
                }
                else if(cell->neighbors >= 4){
                    cell->next_state = 0; 
                    update_neighbors(grid, row, col, -1);
                }
                else{
                    cell->next_state = 1;
                }
            }
        }
    }
}

void placeAliveCell(CellGrid *grid, int row, int col){
    Cell *cell = &grid->rows[row].items[col];
    if(cell->current_state == 1){
        return;
    }
    cell->current_state = 1;

    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            // Check if the neighboring cell is within the valid range
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS) {
                if(i == row && j == col){
                    continue;
                }
                // Modify the neighbor's value
                grid->rows[i].items[j].neighbors += 1;
                grid->rows[i].items[j].next_neighbors += 1;
            }
        }
    }
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);

    InitWindow(1920, 1080, "Game Of Life");
    MaximizeWindow();

    CellGrid grid;
    grid.rows = malloc(sizeof(CellRow) * ROWS);
    grid.size = ROWS;

    float zoomLevel = 2.0;
    Vector2 gridPosition = {.x = GetScreenWidth()/2 - ((COLS*zoomLevel)/2), .y=GetScreenHeight()/2 - ((ROWS*zoomLevel)/2)};
    bool playing = 0;

    Font infoPanelFont = LoadFont("resources/liberation_mono/LiberationMono-MW1v.ttf");
    Vector2 infoPanelSize = {.x=445, .y=130};
    Vector2 infoPanelPosition = {.x=20, .y=GetScreenHeight()-infoPanelSize.y-20};
    const char *infoPanelText = "Conway's Game of Life \n\n\
    - Use scroll wheel to resize the Grid (CTRL speed)\n\
    - Use left click to place a Cell\n\
    - Use right click to center the Grid on cursor\n\
    - Press SPACE for the next generation\n\
    - Press P to toggle AutoPlay";

    Vector2 playingButtonPosition = {0};

    for (size_t i = 0; i < ROWS; ++i) {
        grid.rows[i].items = malloc(sizeof(Cell) * COLS);
        grid.rows[i].size = COLS;

        for (size_t j = 0; j < COLS; ++j) {
            grid.rows[i].items[j].current_state = 0;
            grid.rows[i].items[j].next_state = 0;
            grid.rows[i].items[j].neighbors = 0;
            grid.rows[i].items[j].next_neighbors = 0;
        } 
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(colorBackground);

        // Main grid
        drawMainGrid(&grid, &gridPosition, &zoomLevel);

        // Draw Info Panel
        DrawRectangleV(infoPanelPosition, infoPanelSize, colorInfoPanel);
        DrawTextEx(infoPanelFont, infoPanelText, Vector2Add(infoPanelPosition, (Vector2){.x=10, .y=10}), 15, 0.3, BLACK);
        
        playingButtonPosition = Vector2Add(infoPanelPosition, (Vector2){.x=200, .y=10});
        drawPlayingButton(&playingButtonPosition, &playing, &infoPanelFont);

        EndDrawing();

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            Vector2 gridSize = {.x=COLS*zoomLevel, .y=ROWS*zoomLevel};
            Vector2 mouse = GetMousePosition();
            Vector2 mouseOnGrid = Vector2Subtract(mouse, gridPosition);
            Vector2 t = Vector2Subtract(gridSize, mouseOnGrid);

            if(mouseOnGrid.x < 0 || mouseOnGrid.y < 0 || t.x < 0 || t.y < 0){
                ;
            }else {
                placeAliveCell(&grid, (int)(mouseOnGrid.y/zoomLevel), (int)(mouseOnGrid.x/zoomLevel));
            }
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            Vector2 halfGridSize = {.x=(COLS*zoomLevel)/2, .y=(ROWS*zoomLevel)/2};
            gridPosition = Vector2Subtract(GetMousePosition(), halfGridSize);
        }

        if(IsKeyPressed(KEY_SPACE)){
            generate_next(&grid);
            apply_next(&grid);
        }

        if(playing){
            generate_next(&grid);
            apply_next(&grid);
        }

        if(GetMouseWheelMove() > 0){
            if(IsKeyDown(KEY_LEFT_CONTROL)){
                zoomLevel += 1;
            }
            zoomLevel += 0.1;
        }
        if(GetMouseWheelMove() < 0){
            if(zoomLevel > 1){

                if(IsKeyDown(KEY_LEFT_CONTROL)){
                    zoomLevel -= 1;
                }
                zoomLevel -= 0.1;
            }
        }
        if(IsKeyPressed(KEY_P)){
            playing = !playing;
        }

        if(IsWindowResized()){
            infoPanelPosition = (Vector2){.x=20, .y=GetScreenHeight()-infoPanelSize.y-20};
        } 
    }

    for (size_t i = 0; i < ROWS; ++i) {
        free(grid.rows[i].items);
    }
        
    free(grid.rows);

    CloseWindow();

    return 0;
}
