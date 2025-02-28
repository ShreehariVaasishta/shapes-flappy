/**
 * Working Flappy Bird Game
 * Built incrementally from the minimal working version
 *
 * Compilation (EndeavourOS/Arch):
 * gcc -o flappy_bird flappy_bird.c -I/usr/include/SDL2 -lSDL2 -lm
 */

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// Window dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Game constants
#define BIRD_WIDTH 40
#define BIRD_HEIGHT 30
#define GRAVITY 0.4
#define JUMP_FORCE -8.0
#define PIPE_WIDTH 60
#define PIPE_GAP 170
#define PIPE_SPEED 3
#define MAX_PIPES 10
#define PIPE_SPAWN_TIME 1500 // milliseconds

// Game structures
typedef struct
{
    float x, y;
    float velocity;
    SDL_Rect rect;
} Bird;

typedef struct
{
    int x;
    int gap_y;
    bool passed;
    SDL_Rect top_rect;
    SDL_Rect bottom_rect;
} Pipe;

// Function prototypes
void create_pipe();
bool check_collision(SDL_Rect a, SDL_Rect b);
void update_game();
void render_game(SDL_Renderer *renderer);
void reset_game();

// Global variables
Bird bird;
Pipe pipes[MAX_PIPES];
int next_pipe = 0;
bool game_over = false;
int score = 0;
Uint32 last_pipe_time = 0;

int main(int argc, char *args[])
{
    printf("Starting Flappy Bird...\n");

    // Seed random number generator
    srand(time(NULL));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Flappy Bird",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (renderer == NULL)
    {
        fprintf(stderr, "Software renderer failed, trying accelerated...\n");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if (renderer == NULL)
        {
            fprintf(stderr, "All renderers failed! SDL_Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }

    // Initialize game state
    reset_game();

    // Main game loop
    bool quit = false;
    SDL_Event e;

    // First render to show initial state
    render_game(renderer);

    Uint32 last_time = SDL_GetTicks();

    while (!quit)
    {
        // Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_SPACE:
                    if (game_over)
                    {
                        reset_game();
                    }
                    else
                    {
                        bird.velocity = JUMP_FORCE;
                    }
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    if (game_over)
                    {
                        reset_game();
                    }
                    else
                    {
                        bird.velocity = JUMP_FORCE;
                    }
                }
            }
        }

        // Update game state
        if (!game_over)
        {
            update_game();
        }

        // Render
        render_game(renderer);

        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Game exited cleanly.\n");
    return 0;
}

void create_pipe()
{
    Pipe new_pipe;
    new_pipe.x = SCREEN_WIDTH;

    // Ensure gap is within screen bounds
    int min_gap_y = PIPE_GAP / 2 + 50;
    int max_gap_y = SCREEN_HEIGHT - PIPE_GAP / 2 - 50;
    new_pipe.gap_y = min_gap_y + rand() % (max_gap_y - min_gap_y);

    new_pipe.passed = false;

    new_pipe.top_rect.x = new_pipe.x;
    new_pipe.top_rect.y = 0;
    new_pipe.top_rect.w = PIPE_WIDTH;
    new_pipe.top_rect.h = new_pipe.gap_y - PIPE_GAP / 2;

    new_pipe.bottom_rect.x = new_pipe.x;
    new_pipe.bottom_rect.y = new_pipe.gap_y + PIPE_GAP / 2;
    new_pipe.bottom_rect.w = PIPE_WIDTH;
    new_pipe.bottom_rect.h = SCREEN_HEIGHT - new_pipe.bottom_rect.y;

    pipes[next_pipe] = new_pipe;
    next_pipe = (next_pipe + 1) % MAX_PIPES;
}

bool check_collision(SDL_Rect a, SDL_Rect b)
{
    // Check if two rectangles are colliding
    int left_a = a.x;
    int right_a = a.x + a.w;
    int top_a = a.y;
    int bottom_a = a.y + a.h;

    int left_b = b.x;
    int right_b = b.x + b.w;
    int top_b = b.y;
    int bottom_b = b.y + b.h;

    if (bottom_a <= top_b)
        return false;
    if (top_a >= bottom_b)
        return false;
    if (right_a <= left_b)
        return false;
    if (left_a >= right_b)
        return false;

    return true;
}

void update_game()
{
    // Check if it's time to spawn a new pipe
    Uint32 current_time = SDL_GetTicks();
    if (current_time - last_pipe_time > PIPE_SPAWN_TIME)
    {
        create_pipe();
        last_pipe_time = current_time;
    }

    // Update bird position
    bird.velocity += GRAVITY;
    bird.y += bird.velocity;
    bird.rect.y = (int)bird.y;

    // Check for collision with ceiling
    if (bird.rect.y < 0)
    {
        bird.rect.y = 0;
        bird.y = 0;
        bird.velocity = 0;
    }

    // Check for collision with ground
    if (bird.rect.y + bird.rect.h > SCREEN_HEIGHT - 20)
    {
        game_over = true;
    }

    // Update pipes and check for collisions
    for (int i = 0; i < MAX_PIPES; i++)
    {
        // Skip pipes that are way off-screen
        if (pipes[i].x > SCREEN_WIDTH + 100)
            continue;

        // Update pipe position
        pipes[i].x -= PIPE_SPEED;
        pipes[i].top_rect.x = pipes[i].x;
        pipes[i].bottom_rect.x = pipes[i].x;

        // Check if bird passed the pipe
        if (!pipes[i].passed && pipes[i].x + PIPE_WIDTH < bird.rect.x)
        {
            pipes[i].passed = true;
            score++;
        }

        // Check for collision with pipes
        if (check_collision(bird.rect, pipes[i].top_rect) ||
            check_collision(bird.rect, pipes[i].bottom_rect))
        {
            game_over = true;
        }
    }
}

void render_game(SDL_Renderer *renderer)
{
    // Clear screen with sky blue
    SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);
    SDL_RenderClear(renderer);

    // Draw pipes (green)
    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i].x + PIPE_WIDTH > 0 && pipes[i].x < SCREEN_WIDTH)
        {
            SDL_RenderFillRect(renderer, &pipes[i].top_rect);
            SDL_RenderFillRect(renderer, &pipes[i].bottom_rect);
        }
    }

    // Draw ground (brown)
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    SDL_Rect ground = {0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20};
    SDL_RenderFillRect(renderer, &ground);

    // Draw bird (yellow)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &bird.rect);

    // Draw game over indicator (red rectangle in center)
    if (game_over)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect message_rect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, 200, 60};
        SDL_RenderFillRect(renderer, &message_rect);
    }

    // Display score as a number in the corner of the screen
    // Since we don't have SDL_ttf, we'll draw a simple digit display using rectangles

    // Draw score in top-left corner
    int score_display = score;
    int digit_width = 20;
    int digit_spacing = 5;
    int x_position = 20;

    // Handle score of 0 specially
    if (score_display == 0)
    {
        // Draw 0 using rectangles (simple 7-segment style)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Top horizontal
        SDL_Rect top = {x_position, 20, digit_width, 4};
        SDL_RenderFillRect(renderer, &top);

        // Bottom horizontal
        SDL_Rect bottom = {x_position, 20 + digit_width, digit_width, 4};
        SDL_RenderFillRect(renderer, &bottom);

        // Left vertical
        SDL_Rect left = {x_position, 20, 4, digit_width + 4};
        SDL_RenderFillRect(renderer, &left);

        // Right vertical
        SDL_Rect right = {x_position + digit_width - 4, 20, 4, digit_width + 4};
        SDL_RenderFillRect(renderer, &right);
    }

    // Draw each digit of the score
    while (score_display > 0)
    {
        int digit = score_display % 10;
        score_display /= 10;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Draw the digit based on its value
        switch (digit)
        {
        case 0:
            // Top horizontal
            SDL_RenderDrawLine(renderer, x_position, 20, x_position + digit_width, 20);
            // Bottom horizontal
            SDL_RenderDrawLine(renderer, x_position, 20 + digit_width, x_position + digit_width, 20 + digit_width);
            // Left vertical
            SDL_RenderDrawLine(renderer, x_position, 20, x_position, 20 + digit_width);
            // Right vertical
            SDL_RenderDrawLine(renderer, x_position + digit_width, 20, x_position + digit_width, 20 + digit_width);
            break;

        case 1:
            // Right vertical
            SDL_RenderDrawLine(renderer, x_position + digit_width, 20, x_position + digit_width, 20 + digit_width);
            break;

        case 2:
            // Top horizontal
            SDL_RenderDrawLine(renderer, x_position, 20, x_position + digit_width, 20);
            // Middle horizontal
            SDL_RenderDrawLine(renderer, x_position, 20 + digit_width / 2, x_position + digit_width, 20 + digit_width / 2);
            // Bottom horizontal
            SDL_RenderDrawLine(renderer, x_position, 20 + digit_width, x_position + digit_width, 20 + digit_width);
            // Top-right vertical
            SDL_RenderDrawLine(renderer, x_position + digit_width, 20, x_position + digit_width, 20 + digit_width / 2);
            // Bottom-left vertical
            SDL_RenderDrawLine(renderer, x_position, 20 + digit_width / 2, x_position, 20 + digit_width);
            break;

        // Add cases 3-9 if needed
        default:
            // For simplicity, just draw a rectangle for other digits
            SDL_Rect digit_rect = {x_position, 20, digit_width, digit_width};
            SDL_RenderFillRect(renderer, &digit_rect);
            break;
        }

        // Move position for next digit (to the left)
        x_position += digit_width + digit_spacing;
    }

    // Also output score to console when it changes
    static int last_score = 0;
    if (score != last_score)
    {
        printf("Score: %d\n", score);
        last_score = score;
    }

    // Update screen
    SDL_RenderPresent(renderer);
}

void reset_game()
{
    // Initialize bird
    bird.x = SCREEN_WIDTH / 4;
    bird.y = SCREEN_HEIGHT / 2;
    bird.velocity = 0;
    bird.rect.x = (int)bird.x;
    bird.rect.y = (int)bird.y;
    bird.rect.w = BIRD_WIDTH;
    bird.rect.h = BIRD_HEIGHT;

    // Initialize pipes
    for (int i = 0; i < MAX_PIPES; i++)
    {
        pipes[i].x = SCREEN_WIDTH * 2; // Position off-screen
    }

    // Reset game state
    game_over = false;
    score = 0;
    last_pipe_time = SDL_GetTicks();

    printf("Game reset. Score: 0\n");
}