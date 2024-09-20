#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define GRID_SIZE 4
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400


typedef struct {
    int x, y;
} Position;
typedef struct {
    SDL_Texture* texture;
} Tile;

Tile tile_textures[GRID_SIZE * GRID_SIZE];
SDL_Texture* original_image_texture = NULL;


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect grid_rect[GRID_SIZE][GRID_SIZE];
int grid[GRID_SIZE][GRID_SIZE];
Position blank_pos;

SDL_Color text_color = {255, 255, 255};  
void load_tile_images() {
    // Load images for each tile
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        char filename[20];
        snprintf(filename, sizeof(filename), "images/img%d.jpg", i+1);
        SDL_Surface* surface = IMG_Load(filename);

        if (surface == NULL) {
            printf("Error loading image: %s\n", IMG_GetError());
            SDL_Quit();
            exit(1);
        }

        tile_textures[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    // Load the original image
    char original_filename[20];
    snprintf(original_filename, sizeof(original_filename), "original.jpg");
    SDL_Surface* original_surface = IMG_Load(original_filename);
    if (original_surface == NULL) {
        printf("Error loading original image: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }

    original_image_texture = SDL_CreateTextureFromSurface(renderer, original_surface);
    SDL_FreeSurface(original_surface);
}


void render_grid() {
    int i, j;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &grid_rect[i][j]);

            if (grid[i][j] != 0) {
                int tile_index = grid[i][j] - 1;
                SDL_Rect dst = {grid_rect[i][j].x, grid_rect[i][j].y, 100, 100};
                SDL_RenderCopy(renderer, tile_textures[tile_index].texture, NULL, &dst);
            }
        }
    }

    // Render the original image on the right side
    SDL_Rect original_dst = {(GRID_SIZE * 100)+100, 100, 200, 200};
    SDL_RenderCopy(renderer, original_image_texture, NULL, &original_dst);
}

void swap_tiles(Position* pos1, Position* pos2) {
    int temp = grid[pos1->y][pos1->x];
    grid[pos1->y][pos1->x] = grid[pos2->y][pos2->x];
    grid[pos2->y][pos2->x] = temp;

    blank_pos.x = pos1->x;
    blank_pos.y = pos1->y;
}

int is_valid_move(Position* pos) {
    return ((abs(pos->x - blank_pos.x) == 1 && pos->y == blank_pos.y) ||
            (abs(pos->y - blank_pos.y) == 1 && pos->x == blank_pos.x));
}





int count_inversions() {
    int inversions = 0;
    int i, j, k, l;

    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            for (k = i; k < GRID_SIZE; k++) {
                for (l = (k == i) ? j + 1 : 0; l < GRID_SIZE; l++) {
                    if (grid[i][j] > grid[k][l] && grid[k][l] != 0) {
                        inversions++;
                    }
                }
            }
        }
    }

    return inversions;
}

int is_solvable() {
    int inversions = count_inversions();
    // Adjust for 0-based indexing and check the parity
    return ((GRID_SIZE % 2 == 1) && (inversions % 2 == 0)) || ((GRID_SIZE % 2 == 0) && ((blank_pos.y % 2 == 0) == (inversions % 2 == 1)));
}
int is_solved() {
    int i, j;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] != 0 && grid[i][j] != i * GRID_SIZE + j + 1) {
                return 0;
            }
        }
    }
    return 1;
}



void shuffle_grid() {
    int i, j;
    int flat_grid[GRID_SIZE * GRID_SIZE];

    // Flatten the 2D grid to a 1D array
    int index = 0;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            flat_grid[index++] = grid[i][j];
        }
    }

    // Shuffle the 1D array using Fisher-Yates algorithm
    for (i = GRID_SIZE * GRID_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap
        int temp = flat_grid[i];
        flat_grid[i] = flat_grid[j];
        flat_grid[j] = temp;
    }

    // Unflatten the 1D array back to the 2D grid
    index = 0;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = flat_grid[index++];
        }
    }

    // Find the position of the blank space
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                blank_pos.x = j;
                blank_pos.y = i;
                return;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("15 Puzzle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/liberation2/LiberationSerif-Regular.ttf", 24); 

    if (font == NULL) {
        printf("Failed to load font! SDL_Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    } else {
        printf("Font loaded successfully!\n");
    }

    int i, j;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            grid_rect[i][j] = (SDL_Rect){j * 100, i * 100, 100, 100};
        }
    }

    srand(time(NULL));
    int index = 0;
    for (i = 0; i < GRID_SIZE; i++) {
        for (j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = ++index;
        }
    }
    grid[GRID_SIZE - 1][GRID_SIZE - 1] = 0;
    blank_pos.x = GRID_SIZE - 1;
    blank_pos.y = GRID_SIZE - 1;

    
    shuffle_grid();
     if (!is_solvable()) {
        // Reshuffle until a solvable configuration is achieved
        while (!is_solvable()) {
            shuffle_grid();
        }
    }
    load_tile_images();

    // Initial rendering
    render_grid();
    SDL_RenderPresent(renderer);

    int quit = 0;
    SDL_Event e;
    SDL_Point clickPoint;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                clickPoint.x = x;
                clickPoint.y = y;

                for (i = 0; i < GRID_SIZE; i++) {
                    for (j = 0; j < GRID_SIZE; j++) {
                        if (SDL_PointInRect(&clickPoint, &grid_rect[i][j])) {
                            Position clicked_pos = {j, i};
                            if (is_valid_move(&clicked_pos)) {
                                swap_tiles(&clicked_pos, &blank_pos);
                                render_grid();
                                SDL_RenderPresent(renderer);

                                if (is_solved()) {
                                    printf("Congratulations! You have solved the puzzle!\n");
                                    quit = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Add a small delay to reduce CPU usage
        SDL_Delay(10);
    }

    // Add a delay before exiting to see the result
    SDL_Delay(2000);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();


    return 0;
}

