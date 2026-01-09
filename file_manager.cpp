#include "raylib.h"
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>


struct FileInfo {
  std::string name;
  bool isDir;
};

std::vector<FileInfo> GetFiles(std::string path) {
  std::vector<FileInfo> files;
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name == ".")
        continue;
      bool isDir = (ent->d_type == DT_DIR);
      files.push_back({name, isDir});
    }
    closedir(dir);
  }
  return files;
}

int main() {
  InitWindow(800, 600, "LumanovOS Dosya Yöneticisi");
  SetTargetFPS(60);

  std::string currentPath = ".";
  std::vector<FileInfo> files = GetFiles(currentPath);
  int selected = -1;

  Color appleBlue = (Color){0, 122, 255, 255};

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    // Başlık
    DrawRectangle(0, 0, GetScreenWidth(), 60, (Color){245, 245, 247, 255});
    DrawText("Dosya Yöneticisi", 20, 20, 20, BLACK);
    DrawText(currentPath.c_str(), 200, 25, 14, GRAY);

    // Dosya Listesi
    for (int i = 0; i < files.size(); i++) {
      Rectangle itemRect = {20, 80.0f + (i * 40), 760, 35};
      bool hovered = CheckCollisionPointRec(GetMousePosition(), itemRect);

      if (hovered) {
        DrawRectangleRounded(itemRect, 0.2f, 10, (Color){240, 240, 240, 255});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          if (files[i].isDir) {
            currentPath += "/" + files[i].name;
            files = GetFiles(currentPath);
            break;
          }
        }
      }

      // İkon (D: Klasör, F: Dosya)
      DrawCircle(40, 97 + (i * 40), 12, files[i].isDir ? appleBlue : GRAY);
      DrawText(files[i].isDir ? "D" : "F", 35, 90 + (i * 40), 15, WHITE);

      DrawText(files[i].name.c_str(), 70, 90 + (i * 40), 18, BLACK);
    }

    DrawText("Geri gitmek için ESC'ye basın. Kapatmak için Pencereyi Kapatın.",
             20, GetScreenHeight() - 30, 14, appleBlue);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
