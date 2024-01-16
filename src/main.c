#include "raylib.h"

#include <stdio.h>
#include <math.h>
#include <stdbool.h>

const int tileSize = 32;
const int lineWidth = 10;

const int map[10][10] = {
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

struct Player {
    Vector2 position;
    Vector2 direction;
    int rotation; //0 -> 360
};

struct Player player = {
    .position = { 5.0f, 5.0f },
    .direction = { 0.0f, 0.0f },
    .rotation = 0
};

void PlayerUpdate(void) {
    if (IsKeyDown(KEY_W)) {
        player.position.x += player.direction.x * 0.1f;
        player.position.y += player.direction.y * 0.1f;
    }

    if (IsKeyDown(KEY_S)) {
        player.position.x -= player.direction.x * 0.1f;
        player.position.y -= player.direction.y * 0.1f;
    }

    if (IsKeyDown(KEY_A)) {
        player.rotation -= 5;
    }

    if (IsKeyDown(KEY_D)) {
        player.rotation += 5;
    }

    if (player.rotation > 360) {
        player.rotation = 0;
    }

    if (player.rotation < 0) {
        player.rotation = 360;
    }

    player.direction.x = cos(player.rotation * (PI / 180));
    player.direction.y = sin(player.rotation * (PI / 180));
}

void Render2DMap(void) {
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {

            int tileX = x * tileSize;
            int tileY = y * tileSize;

            if (map[y][x] == 1) {
                DrawRectangle(tileX, tileY, tileSize, tileSize, WHITE);
            }
        }
    }
}

void Render2DPlayer(void) {
    DrawCircle(player.position.x * tileSize + tileSize / 2, player.position.y * tileSize + tileSize / 2, 5, RED);
}

bool isHit(Vector2 endPosition) {
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {

            if (map[row][col] == 0) {
                continue;
            }

            int tileX = col * tileSize;
            int tileY = row * tileSize;

            Rectangle tileRect = { tileX, tileY, tileSize, tileSize };
            if (CheckCollisionPointRec(endPosition, tileRect)) {
                return true;
            }
        }
    }

    return false;
}

void Draw2DRays(int fov) {
    int maxRayLength = 300;
    
    // send rays out for each degree of the fov
    for (int rayAngle = -fov / 2; rayAngle < fov / 2; ++rayAngle) {
        // calculate the direction of the ray based on the player's rotation
        Vector2 rayDirection = { cos((player.rotation + rayAngle) * (PI / 180)), sin((player.rotation + rayAngle) * (PI / 180)) };
        Vector2 rayPosition = { player.position.x * tileSize + tileSize / 2, player.position.y * tileSize + tileSize / 2 };

        Vector2 rayEnd;
        for (int rayLength = 0; rayLength < maxRayLength; ++rayLength) {
            rayEnd.x = rayPosition.x + rayDirection.x * rayLength;
            rayEnd.y = rayPosition.y + rayDirection.y * rayLength;

            if (isHit(rayEnd)) {
                break;
            }
        }

        DrawLineEx(rayPosition, rayEnd, 1, GREEN);
    }
}

void Draw3dViewport(int fov) {
    int maxRayLength = 200;
    
    // send rays out for each degree of the fov
    for (int rayAngle = -fov / 2; rayAngle < fov / 2; ++rayAngle) {
        // calculate the direction of the ray based on the player's rotation
        Vector2 rayDirection = { cos((player.rotation + rayAngle) * (PI / 180)), sin((player.rotation + rayAngle) * (PI / 180)) };
        Vector2 rayPosition = { player.position.x * tileSize + tileSize / 2, player.position.y * tileSize + tileSize / 2 };
        
        Vector2 rayEnd;
        for (int rayLength = 0; rayLength < maxRayLength; ++rayLength) {
            rayEnd.x = rayPosition.x + rayDirection.x * rayLength;
            rayEnd.y = rayPosition.y + rayDirection.y * rayLength;

            if (isHit(rayEnd)) {
                float distance = sqrt(pow(rayEnd.x - rayPosition.x, 2) + pow(rayEnd.y - rayPosition.y, 2));
                Color wallColor = { 255, 255, 255, 255 - (distance / maxRayLength) * 255 };

                // the rectangle is shorter the further away the wall is
                DrawRectangle(
                    rayAngle * lineWidth + 400 - lineWidth / 2,
                    300 - (maxRayLength / distance) * 100 / 2,
                    lineWidth,
                    (maxRayLength / distance) * 100,
                    wallColor
                );

                break;
            }
        }
    }
}

int main(void) {
    int screenWidth = 800; 
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raycaster");
    SetTargetFPS(30);

    RenderTexture2D topDownTexture = LoadRenderTexture(10 * tileSize, 10 * tileSize);

    while (!WindowShouldClose()) {

        float deltaTime = GetFrameTime();
        float deltaTimeMs = deltaTime * 1000.0f;

        char *title = TextFormat("Raycaster - %.2f ms", deltaTimeMs);
        SetWindowTitle(title);

        PlayerUpdate();

        BeginTextureMode(topDownTexture);
            ClearBackground(BLACK);
            Render2DMap();
            Render2DPlayer();
            Draw2DRays(60);
        EndTextureMode();

        // flip the texture so it's not upside down 
        Rectangle sourceRec = { 0.0f, 0.0f, (float)topDownTexture.texture.width, (float)-topDownTexture.texture.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)topDownTexture.texture.width / 2, (float)topDownTexture.texture.height / 2 };

        BeginDrawing();
            ClearBackground(BLACK);
            Draw3dViewport(60);

            DrawTexturePro(topDownTexture.texture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(topDownTexture);

    CloseWindow();
    return 0;
}
