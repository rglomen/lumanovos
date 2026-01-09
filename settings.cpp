#include "raylib.h"
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


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

void SetWallpaper(std::string path) {
  std::ofstream f(".wallpaper_cfg");
  f << path;
  f.close();
}

int main() {
  InitWindow(700, 550, "LumanovOS Ayarlar");
  SetTargetFPS(60);

  std::vector<std::string> wallpapers = GetWallpapers();
  int selectedTab = 0; // 0: Genel, 1: Arka Plan

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground((Color){245, 245, 247, 255});

    // Yan Menu
    DrawRectangle(0, 0, 180, GetScreenHeight(), (Color){230, 230, 230, 255});

    if (CheckCollisionPointRec(GetMousePosition(),
                               (Rectangle){0, 80, 180, 40}) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      selectedTab = 0;
    if (CheckCollisionPointRec(GetMousePosition(),
                               (Rectangle){0, 120, 180, 40}) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      selectedTab = 1;

    DrawText("AYARLAR", 20, 30, 20, BLACK);
    DrawText("Sistem", 20, 85, 18, (selectedTab == 0 ? BLUE : DARKGRAY));
    DrawText("Arka Plan", 20, 125, 18, (selectedTab == 1 ? BLUE : DARKGRAY));

    // Ana İçerik
    if (selectedTab == 0) {
      DrawText("Sistem Bilgileri", 210, 30, 24, BLACK);
      DrawText("Cihaz: LumanovVM-01", 210, 80, 18, DARKGRAY);
      DrawText("İşletim Sistemi: LumanovOS v1.1", 210, 110, 18, DARKGRAY);
    } else if (selectedTab == 1) {
      DrawText("Arka Plan Seçin", 210, 30, 24, BLACK);

      for (int i = 0; i < wallpapers.size(); i++) {
        Rectangle itemRect = {210, 80.0f + (i * 45), 450, 40};
        bool hovered = CheckCollisionPointRec(GetMousePosition(), itemRect);

        DrawRectangleRounded(itemRect, 0.2f, 10,
                             hovered ? (Color){220, 220, 220, 255} : WHITE);
        DrawRectangleRoundedLines(itemRect, 0.2f, 10, 1,
                                  (Color){200, 200, 200, 255});

        std::string fileName =
            wallpapers[i].substr(wallpapers[i].find_last_of("/") + 1);
        DrawText(fileName.c_str(), 230, 92 + (i * 45), 16, BLACK);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          SetWallpaper(wallpapers[i]);
        }
      }
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
