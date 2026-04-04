#include <raylib.h>
#include <stdio.h>

#define WINDOW_WIDTH 544
#define WINDOW_HEIGHT 600
#define FRAMES_PER_SECOND 60
#define GRID_WIDTH 16
#define GRID_HEIGHT 16
#define FIELD_SIZE 32
#define FIELD_SPACING 2
#define NUM_BOMBS 40
#define TRUE 1
#define FALSE 0
#define NEIGHBOURING_FONT_SIZE 28
#define OFFSET_NUMBER_IN_CELL 8


// States a cell can be in
enum fieldState{
    COVERED,
    FLAGGED,
    OPENED,
    EXPLODED
};


// Object, giving each cell the necessary parameters
struct Cell{
    int numNeighbouring;
    enum fieldState state;
    int mined;
};


// Coordinates in pixels
struct posPx{
    int x;
    int y;
};


// Cell coordinates (not in pixel!)
struct cellCoordinate{
    int x;
    int y;
};


// Initial setup of starting parameters
int gameOver = FALSE;
int firstClick = FALSE;
int score = 0;
int falseFlagged = 0;

struct Cell grid[GRID_WIDTH][GRID_HEIGHT];
struct cellCoordinate firstClickCoordinates;


// Place mines randomly on grid. Check wether cell is already mined. Skip safe zone around firstClickCo
void placeMines(struct cellCoordinate firstClickCo){
    
    int numPlaced = 0;
    while (numPlaced < NUM_BOMBS){
        int x = GetRandomValue(0, GRID_WIDTH - 1);
        int y = GetRandomValue(0, GRID_HEIGHT - 1);
        
        // Skip 3x3 safe zone around first clicked cell
        if (x  >= firstClickCo.x - 1 && x <= firstClickCo.x + 1 && y >= firstClickCo.y - 1 && y <= firstClickCo.y + 1){
            continue;
        }else if (grid[x][y].mined == FALSE){
            grid[x][y].mined = TRUE;
            //printf("%d, %d, \n", x, y);
            numPlaced++;
        }
    }
}


// Calculating the number of neighbouring mines for every cell
void countNeighbouring(){
    for (int x = 0; x < GRID_HEIGHT; x++){
        for (int y = 0; y < GRID_WIDTH; y++){
            
            int neighbouring = 0;
            for (int i = -1; i < 2; i++){
                for (int j = -1; j < 2; j++){

                    if (x + i >= 0 && x + i < GRID_WIDTH && y + j >= 0 && y + j < GRID_HEIGHT && grid[x + i][y + j].mined == TRUE ){
                        if (i == 0 && j == 0){
                            continue;
                        }
                        neighbouring++;
                    }
                }
            }
            grid[x][y].numNeighbouring = neighbouring;
        }
    }
}


// Initial parameters for every cell of the board when game started
void setupGrid(){
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            grid[x][y].state = COVERED;
            grid[x][y].numNeighbouring = 0;
            grid[x][y].mined = FALSE;
        }
    }
}



// Calculates Cell Coordinate from mouse position
struct cellCoordinate getCellCoordinate(){
    
    struct cellCoordinate cell;

    Vector2 mousePosition = GetMousePosition();
    int mousePositionX = (int) mousePosition.x; 
    int mousePositionY = (int) mousePosition.y;

    cell.x = mousePositionX / (FIELD_SIZE + FIELD_SPACING);
    cell.y = mousePositionY / (FIELD_SIZE + FIELD_SPACING);
    
    return cell;
}


// Open all fields with numNeighbouring == 0, around an empty field
void revealEmpty(int x, int y){
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT){
        return;
    }

    if (grid[x][y].state == OPENED || grid[x][y].state == FLAGGED){
        return;
    }

    grid[x][y].state = OPENED;

    if (grid[x][y].mined == TRUE){
        return;
    }

    if (grid[x][y].numNeighbouring > 0){
        return;
    }

    // Open all neighbouring cells using recursion
    for (int i = -1; i <= 1; i++){
        for (int j = -1; j <= 1; j++){
            if (i == 0 && j == 0) continue;
            revealEmpty(x + i, y + j);
        }
    }
}


// Uncovering cells in case gameOver == True, to reveal every mine
void uncoverAllCells(){
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            if (grid[x][y].state != EXPLODED){
                grid[x][y].state = OPENED;
            }
        }
    }
}


// Function for user interaction. If left or right mouse button pressed get pixel coordinate of field and modify state
void mouseOperation(){

    // Right click
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)){

        // Cell is varibale for pixel coordinates
        struct cellCoordinate cell = getCellCoordinate();

        if (grid[cell.x][cell.y].state == COVERED){
            grid[cell.x][cell.y].state = FLAGGED;
            if (grid[cell.x][cell.y].mined == TRUE){
                score++;
            } else {
                falseFlagged++;
            }
                
        }else if (grid[cell.x][cell.y].state == FLAGGED){
            grid[cell.x][cell.y].state = COVERED;
            if (grid[cell.x][cell.y].mined == TRUE){
                score--;
            } else {
                falseFlagged--;
            }
        }
    }



    // Left click
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

        // cell is varibale for pixel coordinates
        struct cellCoordinate cell = getCellCoordinate();

        //first click
        if(firstClick == FALSE){
            firstClick = TRUE;
            placeMines(cell);
            countNeighbouring();
        }

        if (grid[cell.x][cell.y].mined == TRUE && grid[cell.x][cell.y].state == COVERED){
            grid[cell.x][cell.y].state = EXPLODED;
            gameOver = TRUE;
            uncoverAllCells();
        }else if (grid[cell.x][cell.y].state == COVERED){
            revealEmpty(cell.x, cell.y);
        }
    }
}



// Calculates the pixel x and y coordinate for the graphic elements
struct posPx getTexturePos(int xCoordinate, int yCoordinate){
    struct posPx texturePosition;
    texturePosition.x = xCoordinate * FIELD_SIZE + xCoordinate * FIELD_SPACING;
    texturePosition.y = yCoordinate * FIELD_SIZE + yCoordinate * FIELD_SPACING;

    return texturePosition;
}



// Displays the number of neighbouring mines around a cell
void drawNeighbouring(int neighbouring, struct posPx position){
    Color color;
    switch (neighbouring){
        case 1:
            color = RED;
            break;
        case 2:
            color = GREEN;
            break;
        case 3:
            color = BLUE;
            break;
        case 4:
            color = PURPLE;
            break;
        case 5:
            color = BROWN;
            break;
        default:
            color = BLACK;
            break;
    }


    int positionX = position.x + OFFSET_NUMBER_IN_CELL;
    int positionY = position.y + OFFSET_NUMBER_IN_CELL;

    DrawText(TextFormat("%d", neighbouring), positionX, positionY , NEIGHBOURING_FONT_SIZE, color);
}



// Function to place corresponding textures on field
void drawGrid(Texture2D coveredField, Texture2D mineFound, Texture2D flaggedField, Texture2D mineExploded){
    for (int x = 0; x < GRID_WIDTH; x++){
        for (int y = 0; y < GRID_HEIGHT; y++){
            int texturePosX = getTexturePos(x, y).x;
            int texturePosY = getTexturePos(x, y).y;
            
            // Display number of neighbouring mines
            if (grid[x][y].state == OPENED && grid[x][y].mined == FALSE && grid[x][y].numNeighbouring != 0){
                drawNeighbouring(grid[x][y].numNeighbouring, getTexturePos(x, y));
            }
            
            // Draw appropiate texture on field
            switch (grid[x][y].state){
                case COVERED:
                    DrawTexture(coveredField, texturePosX, texturePosY, WHITE);
                    break;
                case FLAGGED:
                    DrawTexture(flaggedField, texturePosX, texturePosY, WHITE);
                    break;
                case OPENED:
                    if(grid[x][y].mined == TRUE){
                        DrawTexture(mineFound, texturePosX, texturePosY, WHITE);
                    }
                    break;
                case EXPLODED:
                    DrawTexture(mineExploded, texturePosX, texturePosY, WHITE);
                    break;
            }
        }
    }
}


// Resets current state of the game into start position
void reset(){
    for (int i = 0; i < GRID_WIDTH; i++){
        for (int j = 0; j < GRID_HEIGHT; j++){
            grid[i][j].mined = FALSE;
            grid[i][j].numNeighbouring = 0;
            grid[i][j].state = COVERED;
        }
    }

    gameOver = FALSE;
    firstClick = FALSE;
    score = 0;
    falseFlagged = 0;
}


// Display of endscreen with user user interface to replay or quit
int drawEndScreen(){

    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.7f));

    // Win or lose message
    if (gameOver == TRUE){
        DrawText("GAME OVER", 80, 100, 60, RED);
    } else if (score == NUM_BOMBS){
        DrawText("YOU WON!", 100, 100, 60, GREEN);
    }

    // Score
    DrawText(TextFormat("Score: %d / %d", score, NUM_BOMBS), 180, 200, 30, WHITE);

    // Replay button
    DrawRectangle(150, 300, 250, 60, DARKGREEN);
    DrawText("RESTART", 190, 315, 30, WHITE);

    // Quti button
    DrawRectangle(150, 400, 250, 60, MAROON);
    DrawText("QUIT", 230, 415, 30, WHITE);
   

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

        // Position and area of replay button
        int replayButX = 150;
        int replayButY = 300;
        int replayButWidth = 250;
        int replayButHeight = 60;

        // Position and area of quit button
        int quitButX = 150;
        int quitButY = 400;
        int quitButWidth = 250;
        int quitButHeight = 60;

        Vector2 mousePos = GetMousePosition();

        // If condition met, replay button pressed
        if (mousePos.x >= replayButX && mousePos.x <= (replayButX + replayButWidth) && mousePos.y >= replayButY && mousePos.y <= replayButY + replayButHeight){
            reset();
            return 1;
        }

        // If condition met, quit button pressed
        if (mousePos.x >= quitButX && mousePos.x <= (quitButX + quitButWidth) && mousePos.y >= quitButY && mousePos.y <= quitButY + quitButHeight){
            return 2;
        }
    }
    return 0;
}

//========================================================================================================================
//====   Main  ===========================================================================================================
//========================================================================================================================

int main(){
    
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minesweeper by Boba");

    SetTargetFPS(FRAMES_PER_SECOND);

    // Load textures  
    Texture2D mineTexture = LoadTexture("../assets/mineFound.png");
    Texture2D mineExploded = LoadTexture("../assets/mineExpolded.png");
    Texture2D flaggedField = LoadTexture("../assets/flaggedField.png");
    Texture2D coveredField = LoadTexture("../assets/coveredField.png");
    Texture2D background = LoadTexture("../assets/backgroundn.png");

    setupGrid();

    while (!WindowShouldClose()){
        BeginDrawing();     
        
        // Setup background and game instructions
        ClearBackground(GRAY);
        DrawTexture(background,0, 0, WHITE);
        DrawText("Left click to open cell | Right click to place a flag", 20, 560, 20, BLACK);
        
        drawGrid(coveredField, mineTexture, flaggedField, mineExploded);
        mouseOperation();

        // Gameover / win condition
        if ((gameOver == TRUE) || (score == NUM_BOMBS && falseFlagged == 0)){
            if (drawEndScreen() == 2){
                break;
            }
        }

        EndDrawing();
    }   

    CloseWindow();
    
    return 0;
}
