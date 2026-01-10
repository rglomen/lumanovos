#include "libs/ui/ui.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>


// Ayar kategorileri
int currentTab = 0;

// Sistem bilgileri
char cpuInfo[256] = "Yukleniyor...";
char memInfo[64] = "Yukleniyor...";
char diskInfo[64] = "Yukleniyor...";
char kernelInfo[64] = "Yukleniyor...";

// Ayar değerleri
float masterVolume = 0.8f;
float brightness = 1.0f;
bool showSeconds = false;
bool autoStart = true;

// Wallpaper listesi
std::vector<std::string> wallpapers;
int currentWallpaper = 0;

void LoadSystemInfo() {
  FILE *fp;

  // CPU
  fp = popen(
      "cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs",
      "r");
  if (fp) {
    if (fgets(cpuInfo, sizeof(cpuInfo), fp)) {
      cpuInfo[strcspn(cpuInfo, "\n")] = 0;
      // Kısalt
      if (strlen(cpuInfo) > 40) {
        cpuInfo[37] = '.';
        cpuInfo[38] = '.';
        cpuInfo[39] = '.';
        cpuInfo[40] = 0;
      }
    }
    pclose(fp);
  }

  // Bellek
  fp = popen("free -h | grep Mem | awk '{print $2}'", "r");
  if (fp) {
    if (fgets(memInfo, sizeof(memInfo), fp)) {
      memInfo[strcspn(memInfo, "\n")] = 0;
    }
    pclose(fp);
  }

  // Disk
  fp = popen("df -h / | tail -1 | awk '{print $2}'", "r");
  if (fp) {
    if (fgets(diskInfo, sizeof(diskInfo), fp)) {
      diskInfo[strcspn(diskInfo, "\n")] = 0;
    }
    pclose(fp);
  }

  // Kernel
  fp = popen("uname -r", "r");
  if (fp) {
    if (fgets(kernelInfo, sizeof(kernelInfo), fp)) {
      kernelInfo[strcspn(kernelInfo, "\n")] = 0;
    }
    pclose(fp);
  }
}

void LoadWallpaperList() {
  wallpapers.clear();

  // images klasöründen
  DIR *dir = opendir("images");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
      std::string name = ent->d_name;
      if (name.find(".jpg") != std::string::npos ||
          name.find(".png") != std::string::npos) {
        wallpapers.push_back("images/" + name);
      }
    }
    closedir(dir);
  }

  // assets/wallpapers klasöründen
  dir = opendir("assets/wallpapers");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
      std::string name = ent->d_name;
      if (name.find(".jpg") != std::string::npos ||
          name.find(".png") != std::string::npos) {
        wallpapers.push_back("assets/wallpapers/" + name);
      }
    }
    closedir(dir);
  }
}

void DrawCustomCursor(Vector2 pos) {
  DrawTriangle(pos, {pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, WHITE);
  DrawLineEx(pos, {pos.x, pos.y + 18}, 1.5f, BLACK);
  DrawLineEx(pos, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
  DrawLineEx({pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(650, 520, "Ayarlar");
  SetTargetFPS(60);
  HideCursor();
  LUI::SetTheme(true);

  LoadSystemInfo();
  LoadWallpaperList();

  const char *tabs[] = {"Gorunum", "Arkaplan", "Ses", "Sistem", "Hakkinda"};
  const int tabCount = 5;

  // Wallpaper önizleme
  Texture2D wallpaperPreview = {0};
  int previewIndex = -1;

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);

    // Pencere çerçevesi
    LUI::WindowResult winResult =
        LUI::DrawWindowFrame({0, 0, (float)w, (float)h}, "Ayarlar", true);
    if (winResult.closeClicked)
      break;

    // Sol menü
    float menuW = 130;
    DrawRectangle(0, 32, menuW, h - 32, LUI::currentTheme->panelBg);

    for (int i = 0; i < tabCount; i++) {
      Rectangle tabRect = {5, 45.0f + i * 42, menuW - 10, 38};
      bool selected = (currentTab == i);

      if (selected) {
        DrawRectangleRounded(tabRect, 0.3f, 8, LUI::currentTheme->accent);
      } else if (CheckCollisionPointRec(mouse, tabRect)) {
        DrawRectangleRounded(tabRect, 0.3f, 8,
                             Fade(LUI::currentTheme->accent, 0.3f));
      }

      DrawText(tabs[i], tabRect.x + 12, tabRect.y + 12, 15,
               selected ? WHITE : LUI::currentTheme->text);

      if (CheckCollisionPointRec(mouse, tabRect) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentTab = i;
      }
    }

    // İçerik alanı
    float contentX = menuW + 20;
    float contentW = w - menuW - 30;
    float y = 55;

    // TAB 0: Görünüm
    if (currentTab == 0) {
      DrawText("Tema", contentX, y, 18, LUI::currentTheme->text);
      y += 35;

      Rectangle darkBtn = {contentX, y, 110, 38};
      Rectangle lightBtn = {contentX + 120, y, 110, 38};

      if (LUI::Button(darkBtn, "Koyu Tema", !LUI::isDarkTheme)) {
        LUI::SetTheme(true);
      }
      if (LUI::Button(lightBtn, "Acik Tema", LUI::isDarkTheme)) {
        LUI::SetTheme(false);
      }
      y += 60;

      DrawText("Accent Renk", contentX, y, 18, LUI::currentTheme->text);
      y += 35;

      Color colors[] = {{0, 122, 255, 255},  {255, 45, 85, 255},
                        {52, 199, 89, 255},  {255, 149, 0, 255},
                        {175, 82, 222, 255}, {255, 204, 0, 255}};
      for (int i = 0; i < 6; i++) {
        Rectangle colorBtn = {contentX + i * 50, y, 44, 44};
        DrawRectangleRounded(colorBtn, 0.3f, 8, colors[i]);

        // Seçili renk göstergesi
        if (colors[i].r == LUI::currentTheme->accent.r &&
            colors[i].g == LUI::currentTheme->accent.g) {
          DrawRectangleRoundedLines(colorBtn, 0.3f, 8, WHITE);
        }

        if (CheckCollisionPointRec(mouse, colorBtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          LUI::currentTheme->accent = colors[i];
        }
      }
      y += 70;

      DrawText("Parlaklik", contentX, y, 18, LUI::currentTheme->text);
      y += 30;
      brightness =
          LUI::Slider({contentX, y, contentW - 80, 18}, brightness, 0.5f, 1.0f);
      DrawText(TextFormat("%d%%", (int)(brightness * 100)),
               contentX + contentW - 60, y, 14, LUI::currentTheme->text);
    }

    // TAB 1: Arkaplan
    else if (currentTab == 1) {
      DrawText("Arkaplan Resmi", contentX, y, 18, LUI::currentTheme->text);
      y += 35;

      if (wallpapers.empty()) {
        DrawText("Arkaplan resmi bulunamadi.", contentX, y, 14,
                 LUI::currentTheme->textDim);
        DrawText("images/ veya assets/wallpapers/", contentX, y + 20, 12,
                 LUI::currentTheme->textDim);
        DrawText("klasorune .jpg veya .png ekleyin.", contentX, y + 35, 12,
                 LUI::currentTheme->textDim);
      } else {
        // Wallpaper listesi
        float thumbSize = 80;
        int cols = 4;

        for (size_t i = 0; i < wallpapers.size() && i < 12; i++) {
          int col = i % cols;
          int row = i / cols;
          Rectangle thumbRect = {contentX + col * (thumbSize + 10),
                                 y + row * (thumbSize + 10), thumbSize,
                                 thumbSize - 10};

          // Seçili wallpaper vurgusu
          if ((int)i == currentWallpaper) {
            DrawRectangleRoundedLines(thumbRect, 0.1f, 6,
                                      LUI::currentTheme->accent);
          }

          // Placeholder (gerçek thumbnail yüklemek için daha fazla kod gerekir)
          DrawRectangleRounded(thumbRect, 0.1f, 6, LUI::currentTheme->border);

          // Dosya adını kısa göster
          std::string filename = wallpapers[i];
          size_t pos = filename.find_last_of('/');
          if (pos != std::string::npos)
            filename = filename.substr(pos + 1);
          if (filename.length() > 10)
            filename = filename.substr(0, 8) + "..";

          DrawText(filename.c_str(), thumbRect.x + 5,
                   thumbRect.y + thumbSize - 25, 10, LUI::currentTheme->text);

          if (CheckCollisionPointRec(mouse, thumbRect) &&
              IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentWallpaper = i;
            // Config dosyasına yaz
            std::ofstream cfg("config/wallpaper.conf");
            if (cfg.is_open()) {
              cfg << wallpapers[i];
              cfg.close();
            }
          }
        }

        y += (wallpapers.size() / cols + 1) * (thumbSize + 10) + 20;

        DrawText("Secilen:", contentX, y, 14, LUI::currentTheme->textDim);
        if (currentWallpaper >= 0 &&
            currentWallpaper < (int)wallpapers.size()) {
          DrawText(wallpapers[currentWallpaper].c_str(), contentX + 60, y, 14,
                   LUI::currentTheme->text);
        }
      }
    }

    // TAB 2: Ses
    else if (currentTab == 2) {
      DrawText("Ses Ayarlari", contentX, y, 18, LUI::currentTheme->text);
      y += 45;

      DrawText("Ana Ses", contentX, y, 15, LUI::currentTheme->text);
      y += 25;
      masterVolume =
          LUI::Slider({contentX, y, contentW - 80, 18}, masterVolume, 0, 1);
      DrawText(TextFormat("%d%%", (int)(masterVolume * 100)),
               contentX + contentW - 60, y, 14, LUI::currentTheme->text);
      y += 50;

      DrawText("Sistem Sesleri", contentX, y, 15, LUI::currentTheme->text);
      y += 30;

      bool notifySound = true;
      LUI::Checkbox({contentX, y, 200, 28}, "Bildirim sesleri", &notifySound);
      y += 35;

      bool clickSound = false;
      LUI::Checkbox({contentX, y, 200, 28}, "Tiklama sesleri", &clickSound);
    }

    // TAB 3: Sistem
    else if (currentTab == 3) {
      DrawText("Sistem Ayarlari", contentX, y, 18, LUI::currentTheme->text);
      y += 45;

      LUI::Checkbox({contentX, y, 250, 28}, "Otomatik baslat", &autoStart);
      y += 40;

      LUI::Checkbox({contentX, y, 250, 28}, "Saatte saniyeleri goster",
                    &showSeconds);
      y += 50;

      LUI::Separator(contentX, y, contentW);
      y += 20;

      DrawText("Sistem Islemleri", contentX, y, 16, LUI::currentTheme->text);
      y += 35;

      if (LUI::Button({contentX, y, 150, 36}, "Yeniden Baslat")) {
        system("echo 120820 | sudo -S reboot");
      }

      if (LUI::Button({contentX + 160, y, 150, 36}, "Sistemi Kapat")) {
        system("echo 120820 | sudo -S poweroff");
      }
    }

    // TAB 4: Hakkında
    else if (currentTab == 4) {
      // Logo
      float cx = contentX + contentW / 2;
      DrawCircleGradient(cx, y + 35, 45, LUI::currentTheme->accent,
                         Fade(LUI::currentTheme->accent, 0.2f));
      DrawText("L", cx - 18, y + 8, 55, WHITE);
      y += 100;

      const char *title = "LumanovOS";
      int tw = MeasureText(title, 32);
      DrawText(title, cx - tw / 2, y, 32, LUI::currentTheme->text);
      y += 40;

      DrawText("Versiyon 1.0.0", cx - MeasureText("Versiyon 1.0.0", 16) / 2, y,
               16, LUI::currentTheme->textDim);
      y += 50;

      LUI::Separator(contentX, y, contentW);
      y += 25;

      DrawText("Sistem Bilgileri", contentX, y, 17, LUI::currentTheme->text);
      y += 35;

      DrawText("Islemci:", contentX, y, 14, LUI::currentTheme->textDim);
      DrawText(cpuInfo, contentX + 80, y, 13, LUI::currentTheme->text);
      y += 26;

      DrawText("Bellek:", contentX, y, 14, LUI::currentTheme->textDim);
      DrawText(memInfo, contentX + 80, y, 14, LUI::currentTheme->text);
      y += 26;

      DrawText("Disk:", contentX, y, 14, LUI::currentTheme->textDim);
      DrawText(diskInfo, contentX + 80, y, 14, LUI::currentTheme->text);
      y += 26;

      DrawText("Kernel:", contentX, y, 14, LUI::currentTheme->textDim);
      DrawText(kernelInfo, contentX + 80, y, 14, LUI::currentTheme->text);
    }

    // Mouse imleci
    DrawCustomCursor(mouse);

    EndDrawing();
  }

  if (wallpaperPreview.id > 0)
    UnloadTexture(wallpaperPreview);
  ShowCursor();
  CloseWindow();
  return 0;
}
