#include "raylib.h"

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>

#define PI 3.14159265358979323846f

const int screenWidth = 1600;
const int screenHeight = 1200;

const int canvasPixelScale = 2;

const int tileSize = 64;
#define ROOM_COLUMNS 10
#define ROOM_ROWS 10
const int map[ROOM_COLUMNS][ROOM_ROWS] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 1, 1, 0, 0, 0, 1 },
    { 1, 1, 1, 0, 1, 0, 0, 0, 0, 1 },
    { 1, 0, 1, 0, 1, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

const float playerMoveSpeed = 3.0f;
const float playerRotationSpeed = 3.0f;
struct Player {
    Vector2 position;
    Vector2 direction;
    float rotation; // in radians
} player = {
    .position = { 3.0f, 3.0f },
    .direction = {
        .x = 1.0f, // cosf(0.0f)
        .y = 0.0f  // sinf(0.0f)
    },
    .rotation = 0.0f
};

float frameDistances[60] = { 0 };

void UpdatePlayer(float deltaTime) {
    if (IsKeyDown(KEY_W)) {
        player.position.x += player.direction.x * playerMoveSpeed * deltaTime;
        player.position.y += player.direction.y * playerMoveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_S)) {
        player.position.x -= player.direction.x * playerMoveSpeed * deltaTime;
        player.position.y -= player.direction.y * playerMoveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_A)) {
        player.rotation -= playerRotationSpeed * deltaTime;

        if (player.rotation < 0.0f) {
            player.rotation += 2.0f * PI;
        }

        player.direction.x = cosf(player.rotation);
        player.direction.y = sinf(player.rotation);
    }
    if (IsKeyDown(KEY_D)) {
        player.rotation += playerRotationSpeed * deltaTime;

        if (player.rotation > 2.0f * PI) {
            player.rotation -= 2.0f * PI;
        }
        
        player.direction.x = cosf(player.rotation);
        player.direction.y = sinf(player.rotation);
    }
}

void DrawPlayer() {
    Vector2 playerPosition = {
        player.position.x * tileSize,
        player.position.y * tileSize
    };
    DrawCircleV(playerPosition, 10, BLUE);
    DrawLineEx(playerPosition, (Vector2){
        playerPosition.x + player.direction.x * 30,
        playerPosition.y + player.direction.y * 30
    }, 2, RED);
}

void DrawMap() {
    for (int x = 0; x < ROOM_COLUMNS; x++) {
        for (int y = 0; y < ROOM_ROWS; y++) {
            Rectangle wall = {
                x * tileSize + 1,
                y * tileSize + 1,
                tileSize - 2,
                tileSize - 2
            };
            Color color = map[x][y] == 1 ? RAYWHITE : BLACK;
            DrawRectangleRec(wall, color);
        }
    }
}

float distance(Vector2 a, Vector2 b) {
    return sqrtf(powf(a.x - b.x, 2) + powf(a.y - b.y, 2));
}

void DrawRays2D() {
    // The player's rotation is in radians
    float rayAngle = player.rotation - PI / 6.0f; // 30 degrees
    Vector2 ray = { 0 };
    Vector2 rayStep = { 0 };

    for (int rayIndex = 0; rayIndex < 60; ++rayIndex) {
        Vector2 shortestRay = { 0 };
        float shortestDistance = FLT_MAX;

        // horizontal ray-grid intersection
        if (rayAngle > PI) {
            ray.y = ((int) player.position.y) * tileSize - 1;
            rayStep.y = -tileSize;
        } else {
            ray.y = ((int) player.position.y) * tileSize + tileSize;
            rayStep.y = tileSize;
        }

        ray.x = player.position.x * tileSize + (ray.y - player.position.y * tileSize) / tanf(rayAngle);
        rayStep.x = rayStep.y / tanf(rayAngle);        

        // looking directly left or right
        if (rayAngle == PI / 2.0f || rayAngle == 3.0f * PI / 2.0f) {
            ray.x = player.position.x * tileSize;
            rayStep.x = 0.0f;
        }

        // ray marching
        while (ray.x >= 0 && ray.x < ROOM_COLUMNS * tileSize && ray.y >= 0 && ray.y < ROOM_ROWS * tileSize) {
            int mapX = (int) (ray.x / tileSize);
            int mapY = (int) (ray.y / tileSize);

            if (map[mapX][mapY] == 1) {
                float distanceToWall = distance(player.position, ray);
                if (distanceToWall < shortestDistance) {
                    shortestDistance = distanceToWall;
                    shortestRay = ray;
                }
                break;
            }

            ray.x += rayStep.x;
            ray.y += rayStep.y;
        }

        // vertical ray-grid intersection
        if (rayAngle > PI / 2.0f && rayAngle < 3.0f * PI / 2.0f) {
            ray.x = ((int) player.position.x) * tileSize - 1;
            rayStep.x = -tileSize;
        } else if (rayAngle < PI / 2.0f || rayAngle > 3.0f * PI / 2.0f) {
            ray.x = ((int) player.position.x) * tileSize + tileSize;
            rayStep.x = tileSize;
        }

        ray.y = player.position.y * tileSize + (ray.x - player.position.x * tileSize) * tanf(rayAngle);
        rayStep.y = rayStep.x * tanf(rayAngle);

        if (rayAngle == 0.0f || rayAngle == PI) {
            ray.y = player.position.y * tileSize;
            rayStep.y = 0.0f;
        }

        // ray marching
        while (ray.x >= 0 && ray.x < ROOM_COLUMNS * tileSize && ray.y >= 0 && ray.y < ROOM_ROWS * tileSize) {
            int mapX = (int) (ray.x / tileSize);
            int mapY = (int) (ray.y / tileSize);

            if (map[mapX][mapY] == 1) {
                float distanceToWall = distance(player.position, ray);
                if (distanceToWall < shortestDistance) {
                    shortestDistance = distanceToWall;
                    shortestRay = ray;
                }
                break;
            }

            ray.x += rayStep.x;
            ray.y += rayStep.y;
        }

        DrawLineEx((Vector2) {
            player.position.x * tileSize,
            player.position.y * tileSize
        }, (Vector2) {
            shortestRay.x,
            shortestRay.y
        }, 2, GREEN);

        rayAngle += PI / 180.0f;
        frameDistances[rayIndex] = shortestDistance;
    }
}

void Draw3D() {

    float maxWallHeight = screenHeight / canvasPixelScale;
    int width = (screenWidth / canvasPixelScale) / 60.0f;

    for (int i = 0; i < 60; ++i) {
        float distance = frameDistances[i];
        float wallHeight = (tileSize / distance) * 1200.0f;

        if (wallHeight > maxWallHeight) {
            wallHeight = maxWallHeight;
        }

        float wallTop = (screenHeight / canvasPixelScale - wallHeight) / 2.0f;
        float wallBottom = wallTop + wallHeight;

        Color color = (Color) {
            .r = 255.0f * (1.0f - distance / 1000.0f),
            .g = 255.0f * (1.0f - distance / 1000.0f),
            .b = 255.0f * (1.0f - distance / 1000.0f),
            .a = 255
        };

        DrawRectangle(i * width, wallTop, width, wallHeight, color);
    }
}

int main(void) {

    InitWindow(screenWidth, screenHeight, "Raycaster");
    SetTargetFPS(30);

    RenderTexture2D topDownTexture = LoadRenderTexture(tileSize * ROOM_COLUMNS + 2, tileSize * ROOM_ROWS + 1);
    Rectangle sourceRec = { 0.0f, 0.0f, (float) topDownTexture.texture.width, (float) -topDownTexture.texture.height };
    Rectangle destRec = { 0.0f, 0.0f, (float)topDownTexture.texture.width / 2, (float) topDownTexture.texture.height / 2 };

    RenderTexture2D canvas = LoadRenderTexture(screenWidth / canvasPixelScale, screenHeight / canvasPixelScale);
    Rectangle canvasSourceRec = { 0.0f, 0.0f, (float) canvas.texture.width, (float) -canvas.texture.height };
    Rectangle canvasDestRec = { 0.0f, 0.0f, (float)canvas.texture.width * canvasPixelScale, (float)canvas.texture.height * canvasPixelScale };

    while (!WindowShouldClose()) {

        float deltaTime = GetFrameTime();
        float deltaTimeMs = deltaTime * 1000.0f;

        char *title = TextFormat("Raycaster - %.2f ms", deltaTimeMs);
        SetWindowTitle(title);

        UpdatePlayer(deltaTime);

        for (int i = 0; i < 60; ++i) {
            frameDistances[i] = 0;
        }

        BeginTextureMode(topDownTexture);
            ClearBackground(GRAY);
            DrawMap();
            DrawPlayer();
            DrawRays2D();
        EndTextureMode();

        BeginTextureMode(canvas);
            ClearBackground(RED);
            Draw3D();
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(canvas.texture, canvasSourceRec, canvasDestRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
            DrawTexturePro(topDownTexture.texture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(topDownTexture);
    UnloadRenderTexture(canvas);

    CloseWindow();
    return 0;
}
