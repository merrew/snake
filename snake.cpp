#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <fstream>

using namespace std;

int cellSize = 25;
int cellCount = 25;

Color Green = { 173, 204, 96, 255 };
Color DarkGreen = { 43, 51, 24, 255 };

const int screenWidth = cellSize * cellCount;
const int screenHeight = cellSize * cellCount;
const int frame_cells = 3;
const int frame = cellSize * frame_cells;

double lastUpdateTime = 0;

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food()
    {
        CreateRandomPos();

        Image image = LoadImage("food.png");
        this->texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    void CreateRandomPos()
    {
        float x = GetRandomValue(frame_cells + 1, cellCount - 2 + frame_cells);
        float y = GetRandomValue(frame_cells + 1, cellCount - 2 + frame_cells);
        this->position = { x, y };
    }

    void Draw()
    {
        DrawTexture(texture, position.x * cellSize, position.y * cellSize, WHITE);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

private:

};

class Snake
{
public:
    deque<Vector2> body;
    Vector2 offset;
    int score;
    Snake()
    {
        body = { Vector2{5, 9},  Vector2{4, 9}, Vector2{3, 9} };
        offset = { 1, 0 };
        score = 0;
    }
    
    void Draw()
    {
        
        for (int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;

           Rectangle segment = Rectangle{ x * cellSize, y * cellSize, (float)cellSize, (float)cellSize };
           DrawRectangleRounded(segment, 0.5, 7, DarkGreen);    
        }
    }

    void Move()
    {
        body.pop_back();
        body.push_front(Vector2Add(body[0], offset));
    }

    bool Check_eat(Food& food)
    {
        if(Vector2Equals(this->body[0], food.position))
        {
            score += 1;
            food.CreateRandomPos();
            body.push_front(Vector2Add(body[0], offset));
            return true;
        }
        //while (!Check_coincidence(food)) {};
        return false;
    }

    bool Check_coincidence(Food& food)
    {
        while (ElementInDeque(food.position, this->body))
        {
            food.CreateRandomPos();
        }
        return true;
    }

    bool Collision_with_tall()
    {
        for (int i = 1; i < body.size(); i++)
        {
            if (Vector2Equals(body[0], body[i]))
            {
                return true;
            }
        }
        return false;
    }
    
    ~Snake()
    {
        
    }
private:
    bool ElementInDeque(Vector2 element, deque<Vector2> deque)
    {
        for (unsigned int i = 0; i < deque.size(); i++)
        {
            if (Vector2Equals(deque[i], element))
            {
                return true;
            }
        }
        return false;
    }
};


class Game
{
public:
    Sound eatSound;
    Sound wallSound;
    
    bool sound_lock = false;
    float level = 0.1;

    Game()
    {
        InitAudioDevice();
        wallSound = LoadSound("game_over.mp3");
        eatSound = LoadSound("eat.mp3");
    }

    bool over()
    {
        if (snake.body[0].x == frame_cells || snake.body[0].x == cellCount - 1 + frame_cells)
        {
            if (!sound_lock) { PlaySound(wallSound); }
            return true;
        }

        if (snake.body[0].y == frame_cells || snake.body[0].y == cellCount - 1 + frame_cells)
        {
            if (!sound_lock) { PlaySound(wallSound); }
            return true;
        }

        if (snake.Collision_with_tall())
        {
            if (!sound_lock) { PlaySound(wallSound); }
            return true;
        }

        return false;
    }

    void Unload_Game()
    {
        snake.body = { Vector2{5, 9},  Vector2{4, 9}, Vector2{3, 9} };
        snake.offset = { 1, 0 };
        snake.score = 0;

        food.CreateRandomPos();
    }

    void Draw()
    {
        snake.Draw();
        food.Draw();
    }

    void Check_Change_Move()
    {
        if (IsKeyPressed(KEY_RIGHT) && snake.offset != Vector2{ -1, 0 })
        {
            snake.offset = { 1, 0 };
        }

        if (IsKeyPressed(KEY_LEFT) && snake.offset != Vector2{ 1, 0 })
        {
            snake.offset = { -1, 0 };
        }

        if (IsKeyPressed(KEY_UP) && snake.offset != Vector2{ 0, 1 })
        {
            snake.offset = { 0, -1 };
        }

        if (IsKeyPressed(KEY_DOWN) && snake.offset != Vector2{ 0, -1 })
        {
            snake.offset = { 0, 1 };
        }
    }

    void Proccess()
    {
        if (snake.Check_eat(food) == true)
        {
            if (!sound_lock) { PlaySound(eatSound); }
        }

        if (eventTriggered(level))
        {
            snake.Move();
        }

        Check_Change_Move();
        
    }

    int Score() { return snake.score; }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

private:
    Food food;
    Snake snake;

    bool eventTriggered(double interval)
    {
        double currentTime = GetTime();
        if (currentTime - lastUpdateTime >= interval)
        {
            lastUpdateTime = currentTime;
            return true;
        }
        else
        {
            return false;
        }
    }
};

class Settings
{
public:
    Texture2D sound_texture;
    Rectangle soundRect;
    bool sound_on = true;

    int level_Set = 2;
    Rectangle levelRect;
    string level_text = "medium";

    Settings()
    {
        Image image = LoadImage("sound_on.png");
        this->sound_texture = LoadTextureFromImage(image);
        soundRect = Rectangle{ ((float)screenWidth + 2 * frame) / 2 - 60, (float)screenHeight + frame + 5, (float)image.width, (float)image.height };

        levelRect = Rectangle{ float(screenWidth + 2 * frame - 40 * 5), (float)screenHeight + frame + 5, 40 * 3, 40 };
        UnloadImage(image);


    }
    void Check_Change(Game &game)
    {
        bool clicked_sound = false;
        bool clicked_Level = false;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();

            if (CheckCollisionPointRec(mousePosition, soundRect)) {
                clicked_sound = true;
            }

            if (CheckCollisionPointRec(mousePosition, levelRect)) {
                //DrawText("CLICK", frame, frame - 40, 40, DarkGreen);
                clicked_Level = true;

            }
        }
        if (clicked_sound)
        {
            if (sound_on)
            {
                Image image = LoadImage("sound_off.png");
                this->sound_texture = LoadTextureFromImage(image);
                UnloadImage(image);
                sound_on = false;
            }
            else
            {
                Image image = LoadImage("sound_on.png");
                this->sound_texture = LoadTextureFromImage(image);
                UnloadImage(image);
                sound_on = true;
            }
            //DrawText("Click", frame, frame - 40, 40, DarkGreen);
            game.sound_lock = !game.sound_lock;
            //clicked = false;
        }

        if (clicked_Level)
        {
            level_Set += 1;
            if (level_Set == 4)
            {
                level_Set = 1;
            }

            switch (level_Set)
            {
            case(1):
                level_text = "easy";
                game.level = 0.2;
                break;
            case(2):
                level_text = "medium";
                game.level = 0.1;
                break;
            case(3):
                level_text = "hard";
                game.level = 0.05;
                break;
            }
        }
        
    }

    void Draw()
    {
        DrawTexture(sound_texture, ((float)screenWidth + 2 * frame) / 2 - 60, (float)screenHeight + frame + 5, WHITE);
        DrawText("Level:", screenWidth + 2 * frame - 40 * 8, (float)screenHeight + frame + 5, 40, DarkGreen);
        DrawText(level_text.c_str(), levelRect.x, levelRect.y, 40, DarkGreen);
    }

    ~Settings()
    {
        UnloadTexture(sound_texture);
    }
};

int main() {

    InitWindow(screenWidth + 2 * frame, screenHeight + 2 * frame, "Snake");
    //DrawRectangleLines(frame, frame, cellSize * cellCount, cellSize * cellCount, DarkGreen);
    SetTargetFPS(60);

    int best_score = 0;

    
    Settings settings;

    Game game;

    bool pause = false;

    while (WindowShouldClose() == false) 
    {
        std::ifstream in("best_score.txt", ios_base::in);
        if (in.is_open())
        {
            in >> best_score;
        }
        in.close();

        BeginDrawing();
        ClearBackground(Green);

        DrawRectangleLines(frame, frame, cellSize * cellCount, cellSize * cellCount, DarkGreen);

        DrawText("Best score:", frame, frame - 40, 40, DarkGreen);
        DrawText(TextFormat("%i", best_score), frame + 40 * 6, frame - 40, 40, DarkGreen);

        DrawText("Score:", frame, screenHeight + frame + 5, 40, DarkGreen);
        DrawText(TextFormat("%i", game.Score()), frame + 45 * 3, screenHeight + frame + 5, 40, DarkGreen);


       settings.Draw();
       settings.Check_Change(game);
       game.Draw();

       if (!pause)
       {
           game.Proccess();
       }
       
       if (game.over())
       {
           while (!IsKeyPressed(KEY_UP) && !IsKeyPressed(KEY_DOWN) && !IsKeyPressed(KEY_RIGHT)) {
               BeginDrawing();
               ClearBackground(Green);

               game.Draw();

               DrawRectangleLines(frame, frame, cellSize * cellCount, cellSize * cellCount, DarkGreen);

               DrawText("Best score:", frame, frame - 40, 40, DarkGreen);
               DrawText(TextFormat("%i", best_score), frame + 40 * 6, frame - 40, 40, DarkGreen);


               DrawText("Score:", frame, screenHeight + frame + 5, 40, DarkGreen);
               DrawText(TextFormat("%i", game.Score()), frame + 45 * 3, screenHeight + frame + 5, 40, DarkGreen);

               settings.Draw();
               settings.Check_Change(game);

               EndDrawing();

               if (WindowShouldClose()) { CloseWindow(); return 0;}
           }
           if (game.Score() > best_score)
           {
               std::ofstream out;
               out.open("best_score.txt");

               out << game.Score();
               out.close();
           }
           game.Unload_Game();
           game.Check_Change_Move();
       }

       if (IsKeyPressed(KEY_SPACE))
       {
           pause = !pause;
       }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}