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
          name.find(".jpg") != std::string::npos)
        images.push_back("images/" + name);
    }
    closedir(dir);
  }
  return images;
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Desktop");
  SetTargetFPS(60);

  // Dark Theme Colors
  Color bgDark = (Color){10, 10, 12, 255};
  Color dockBg = (Color){30, 30, 35, 200};
  Color accentBlue = (Color){0, 122, 255, 255};

  std::vector<std::string> wp = GetWallpapers();
  Texture2D wallpaper = {0};
  if (!wp.empty())
    wallpaper = LoadTexture(wp[0].c_str());

  std::vector<App> dockApps = {
      {"Dosyalar", "D", "./file_manager &", (Color){88, 86, 214, 255}},
      {"Ayarlar", "A", "./settings &", (Color){142, 142, 147, 255}},
      {"Terminal", "T", "xterm &", (Color){0, 0, 0, 255}}};

  while (!WindowShouldClose()) {
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char tStr[10];
    strftime(tStr, sizeof(tStr), "%H:%M", ltm);

    BeginDrawing();
    if (wallpaper.id > 0) {
      DrawTexturePro(
          wallpaper,
          (Rectangle){0, 0, (float)wallpaper.width, (float)wallpaper.height},
          (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
          (Vector2){0, 0}, 0.0f, WHITE);
    } else
      ClearBackground(bgDark);

    // Dock
    float dx = (float)GetScreenWidth() / 2 - 175;
    float dy = (float)GetScreenHeight() - 85;
    DrawRectangleRounded((Rectangle){dx, dy, 350, 70}, 0.4f, 20, dockBg);

    for (int i = 0; i < dockApps.size(); i++) {
      Rectangle r = {dx + 40 + (i * 90), dy + 15, 45, 45};
      bool h = CheckCollisionPointRec(GetMousePosition(), r);
      DrawRectangleRounded(r, 0.3f, 10, h ? DARKGRAY : dockApps[i].color);
      DrawText(dockApps[i].icon, r.x + 15, r.y + 10, 25, WHITE);
      if (h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        system(dockApps[i].command);
    }

    // Top Bar
    DrawRectangle(0, 0, GetScreenWidth(), 30, (Color){0, 0, 0, 100});
    DrawText("LumanovOS", 15, 7, 16, WHITE);
    DrawText(tStr, GetScreenWidth() - 60, 7, 16, WHITE);

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
