#include "libs/ui/ui.h"
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <string>
#include <vector>

struct FileEntry {
  std::string name;
  bool isDir;
};

std::vector<FileEntry> GetDirectoryContents(const std::string &path) {
  std::vector<FileEntry> items;
  DIR *dir = opendir(path.c_str());
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
      if (strcmp(ent->d_name, ".") == 0)
        continue;
      FileEntry entry;
      entry.name = ent->d_name;
      entry.isDir = (ent->d_type == DT_DIR);
      items.push_back(entry);
    }
    closedir(dir);
  }

  // Dizinleri önce sırala
  std::sort(items.begin(), items.end(),
            [](const FileEntry &a, const FileEntry &b) {
              if (a.isDir != b.isDir)
                return a.isDir > b.isDir;
              return a.name < b.name;
            });

  return items;
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(700, 500, "Dosya Yöneticisi");
  SetTargetFPS(60);
  LUI::SetTheme(true);

  std::string currentPath = "/home";
  std::vector<FileEntry> files = GetDirectoryContents(currentPath);
  int selectedIndex = -1;
  float scrollY = 0;

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);

    // Başlık çubuğu
    LUI::WindowResult winResult = LUI::DrawWindowFrame(
        {0, 0, (float)w, (float)h}, "Dosya Yöneticisi", true);
    if (winResult.closeClicked)
      break;

    // Yol gösterici
    DrawRectangle(0, 32, w, 35, LUI::currentTheme->panelBg);
    DrawText(currentPath.c_str(), 15, 42, 14, LUI::currentTheme->text);

    // Geri butonu
    if (currentPath != "/") {
      Rectangle backBtn = {(float)w - 80, 38, 70, 25};
      if (LUI::Button(backBtn, "< Geri")) {
        size_t pos = currentPath.find_last_of('/');
        if (pos == 0)
          currentPath = "/";
        else
          currentPath = currentPath.substr(0, pos);
        files = GetDirectoryContents(currentPath);
        selectedIndex = -1;
        scrollY = 0;
      }
    }

    // Dosya listesi
    Rectangle listArea = {10, 75, (float)w - 20, (float)h - 85};
    DrawRectangleRec(listArea, LUI::currentTheme->background);

    float itemH = 32;
    int visibleItems = listArea.height / itemH;

    // Scroll
    int wheel = GetMouseWheelMove();
    scrollY -= wheel * 3;
    scrollY = LUI::Clamp(scrollY, 0,
                         std::max(0.0f, (float)files.size() - visibleItems));

    for (int i = 0; i < visibleItems && (i + (int)scrollY) < (int)files.size();
         i++) {
      int idx = i + (int)scrollY;
      FileEntry &entry = files[idx];

      Rectangle itemRect = {listArea.x + 5, listArea.y + 5 + i * itemH,
                            listArea.width - 10, itemH - 2};

      const char *icon = entry.isDir ? "📁" : "📄";
      bool selected = (idx == selectedIndex);

      if (LUI::ListItem(itemRect, entry.name.c_str(), selected, icon)) {
        if (entry.isDir) {
          // Dizine gir
          if (entry.name == "..") {
            size_t pos = currentPath.find_last_of('/');
            if (pos == 0)
              currentPath = "/";
            else
              currentPath = currentPath.substr(0, pos);
          } else {
            if (currentPath == "/")
              currentPath = "/" + entry.name;
            else
              currentPath = currentPath + "/" + entry.name;
          }
          files = GetDirectoryContents(currentPath);
          selectedIndex = -1;
          scrollY = 0;
        } else {
          selectedIndex = idx;
        }
      }
    }

    // Scroll bar
    if (files.size() > (size_t)visibleItems) {
      float scrollBarH = listArea.height * visibleItems / files.size();
      float scrollBarY =
          listArea.y + (scrollY / files.size()) * listArea.height;
      DrawRectangleRounded(
          {listArea.x + listArea.width - 8, scrollBarY, 6, scrollBarH}, 0.5f, 8,
          LUI::currentTheme->accent);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
