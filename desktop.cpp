#include "raylib.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>


struct App {
  const char *name;
  const char *icon;
  const char *command;
  Color color;
};

std::vector<std::string> GetWallpapers() {
  std::vector<std::string> images;
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir("images")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name.find(".png") != std::string::npos ||
          name.find(".jpg") != std::string::npos) {
        images.push_back("images/" + name);
      }
    }
    closedir(dir);
  }
  return images;
}

std::string GetCurrentWallpaperPath() {
  std::ifstream f(".wallpaper_cfg");
  std::string path;
  if (f.is_open()) {
    std::getline(f, path);
    f.close();
  }
  return path;
}

int main() {
  InitWindow(1280, 800, "LumanovOS Desktop Environment");
  SetTargetFPS(60);

  Color bgLight = (Color){240, 240, 245, 255};
  Color dockBg = (Color){255, 255, 255, 180};
  Color accentBlue = (Color){0, 122, 255, 255};

  std::vector<std::string> allWallpapers = GetWallpapers();
  std::string currentPath = GetCurrentWallpaperPath();

  if (currentPath.empty() && !allWallpapers.empty()) {
    srand(time(NULL));
    currentPath = allWallpapers[rand() % allWallpapers.size()];
    std::ofstream f(".wallpaper_cfg");
    f << currentPath;
    f.close();
  }

  Texture2D wallpaper = {0};
  if (!currentPath.empty())
    wallpaper = LoadTexture(currentPath.c_str());

  std::vector<App> dockApps = {
      {"Dosyalar", "D", "./file_manager &", (Color){88, 86, 214, 255}},
      {"Ayarlar", "A", "./settings &", (Color){142, 142, 147, 255}},
      {"Terminal", "T", "xterm &", (Color){0, 0, 0, 255}}};

  int checkCounter = 0;

  while (!WindowShouldClose()) {
    // Her 2 saniyede bir ayar dosyasını kontrol et (Basit IPC)
    checkCounter++;
    if (checkCounter % 120 == 0) {
      std::string NewPath = GetCurrentWallpaperPath();
      if (NewPath != currentPath && !NewPath.empty()) {
        if (wallpaper.id > 0)
          UnloadTexture(wallpaper);
        currentPath = NewPath;
        wallpaper = LoadTexture(currentPath.c_str());
      }
    }

    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char timeStr[10], dateStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M", ltm);
    strftime(dateStr, sizeof(dateStr), "%d %b %Y", ltm);

    BeginDrawing();
    if (wallpaper.id > 0) {
      DrawTexturePro(
          wallpaper,
          (Rectangle){0, 0, (float)wallpaper.width, (float)wallpaper.height},
          (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
          (Vector2){0, 0}, 0.0f, WHITE);
    } else {
      ClearBackground(bgLight);
    }

    // Üst Panel
    DrawRectangle(0, 0, GetScreenWidth(), 30, (Color){255, 255, 255, 150});
    DrawText("LumanovOS", 15, 7, 16, BLACK);
    DrawText(timeStr, GetScreenWidth() - 65, 7, 16, BLACK);

    // Dock
    float dockWidth = 350;
    float dockX = (float)GetScreenWidth() / 2 - dockWidth / 2;
    float dockY = (float)GetScreenHeight() - 90;
    DrawRectangleRounded((Rectangle){dockX, dockY, dockWidth, 70}, 0.4f, 20,
                         dockBg);

    for (int i = 0; i < dockApps.size(); i++) {
      Rectangle iconBounds = {dockX + 40 + (i * 90), dockY + 15, 45, 45};
      if (CheckCollisionPointRec(GetMousePosition(), iconBounds)) {
        DrawRectangleRounded(iconBounds, 0.3f, 10, DARKGRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
          system(dockApps[i].command);
      }
      DrawRectangleRounded(iconBounds, 0.3f, 10, dockApps[i].color);
      DrawText(dockApps[i].icon, iconBounds.x + 15, iconBounds.y + 10, 25,
               WHITE);
    }

    EndDrawing();
  }

  if (wallpaper.id > 0)
    UnloadTexture(wallpaper);
  CloseWindow();
  return 0;
}
