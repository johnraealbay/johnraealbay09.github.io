#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 } // Green
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 } // Dark Green

//Size of each cell and number of cells
int cellSize = 25;
int cellCount = 20;
int offset = 75; // Offset for drawing the snake segments

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element,deque<Vector2> deque)
{
    for(unsigned int i =0; i < deque.size(); i++)
    {
        if(Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Snake class
class Snake {
    public:
        deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        Vector2 direction = { 1, 0 };
        bool addSegment = false;

        void Draw() 
        {
            for (unsigned int i = 0; i < body.size(); i++) 
            {
                float x = body[i].x;
                float y = body[i].y;
                Rectangle segment =Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
                DrawRectangleRounded(segment, 0.5f, 6, DARKGREEN);
            }
        }

        Vector2 GetNewHead(Vector2 currentHead, Vector2 direction)
        {
            return Vector2Add(currentHead, direction);
        }

        void Update() 
        {
            // Update snake position
            body.push_front(GetNewHead(body[0], direction));
            if (addSegment == true)
            {
                addSegment = false;
            }
            else
            {
                body.pop_back();
            }

        }

        void Reset()
        {
            body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
            direction = { 1, 0 };
            addSegment = false;
        }
};

// Food class
class Food {

    public:
        Vector2 position = { 5, 6 };
        Texture2D texture;

        Food(deque<Vector2> snakeBody) 
        {
           Image image = LoadImage("graphics/apple.png");
           ImageResize(&image, 23, 23);
           texture = LoadTextureFromImage(image);
           UnloadImage(image);
           position = GenerateRandomsPosition(snakeBody);
        }

        void Draw() 
        {
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        Vector2 GenerateRandomCell() 
        {
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{ x, y };
        }

        Vector2 GenerateRandomsPosition(deque<Vector2> snakeBody) 
        {
            Vector2 position = GenerateRandomCell();
            while(ElementInDeque(position, snakeBody))
            {
                position = GenerateRandomCell();
            }
            return position;
        }
};

// Game class
class Game
{
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        bool running = true;
        int score = 0;
        int highscore = 0;
        Sound eatSound; 
        Sound wallSound;

        Game()
        {   
            // Initialize audio device and load sounds
            InitAudioDevice();
            eatSound = LoadSound("sounds/eat-sound.mp3");
            wallSound = LoadSound("sounds/wall-sound.mp3");
        }

        ~Game()
        {   
            // Unload sounds and close audio device
            UnloadSound(eatSound);
            UnloadSound(wallSound);
            CloseAudioDevice();
        }


        void Draw() 
        {
            food.Draw();
            snake.Draw();
        }

        void Update() 
        {
            if(running)
            {
                snake.Update();
                CheckCollisionWithFood();
                CheckCollisionWithWalls();
                CheckCollisionWithTail();
            }

        }

        void CheckCollisionWithFood() 
        {
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomsPosition(snake.body);
                snake.addSegment = true; // Grow the snake
                score ++;
                PlaySound(eatSound);
            }
        }

        void CheckCollisionWithWalls()
        {
            // Game over if snake hits walls
            if(snake.body[0].x == cellCount || snake.body[0].x == -1)
            {
                GameOver();
            }
            if(snake.body[0].y == cellCount || snake.body[0].y == -1)
            {
                GameOver();
            }
        }

        void GameOver()
        {
            if(score > highscore)
            {
                highscore = score;
            }

            // Game over if snake hits itself
            snake.Reset();
            food.position = food.GenerateRandomsPosition(snake.body);
            running = false;
            score = 0;
            PlaySound(wallSound);
        }

        void CheckCollisionWithTail()
        {
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if(ElementInDeque(snake.body[0], headlessBody))
            {
                GameOver();
            }
        }
};

int main()
{   
    // Initialization of the game
    cout << "Starting game..." << endl;
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        if (eventTriggered(0.2))
        {
            game.Update();
        }

        // Input handling
        if((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && game.snake.direction.x != -1)
        {
            game.snake.direction = { 1, 0 };
            game.running = true;
        }
        if((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && game.snake.direction.x != 1)
        {
            game.snake.direction = { -1, 0 };
            game.running = true;
        }
        if((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && game.snake.direction.y != 1)
        {
            game.snake.direction = { 0, -1 };
            game.running = true;
        }
        if((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && game.snake.direction.y != -1)
        {
            game.snake.direction = { 0, 1 };
            game.running = true;
        }
            // Drawing 
            ClearBackground(GREEN);
            DrawRectangleLinesEx(Rectangle{(float)offset - 2, (float)offset - 2, (float)cellSize * cellCount + 4, (float)cellSize * cellCount + 4}, 5.0f, DARKGREEN); // Border Size
            DrawText("Snake game", offset - 5, 20, 30, DARKGREEN); // Title
            DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount + 10, 30, DARKGREEN); // Score display
            int textWidth = MeasureText(TextFormat("High Score: %i", game.highscore), 30);
            DrawText(TextFormat("High Score: %i", game.highscore),  GetScreenWidth() - textWidth - offset + 5, offset + cellSize * cellCount + 10, 30, DARKGREEN); // High Score display
            game.Draw();

        EndDrawing();
    }
    

    CloseWindow();
    return 0;
}