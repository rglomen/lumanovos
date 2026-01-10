#include "libs/ui/ui.h"
#include <cstdio>
#include <cstring>
#include <fstream>

// Ayar kategorileri
int currentTab = 0;

// Sistem bilgileri
char cpuInfo[256] = "Yükleniyor...";
char memInfo[64] = "Yükleniyor...";
char diskInfo[64] = "Yükleniyor...";
char kernelInfo[64] = "Yükleniyor...";

// Ayar değerleri
float masterVolume = 0.8f;
float musicVolume = 0.7f;
float effectVolume = 0.9f;

void LoadSystemInfo() {
  FILE *fp;

  // CPU
  fp = popen("cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2",
             "r");
  if (fp) {
    if (fgets(cpuInfo, sizeof(cpuInfo), fp)) {
      cpuInfo[strcspn(cpuInfo, "\n")] = 0;
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

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(600, 480, "Ayarlar");
  SetTargetFPS(60);
  LUI::SetTheme(true);

  LoadSystemInfo();

  const char *tabs[] = {"Görünüm", "Ses", "Hakkında"};
  const int tabCount = 3;

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
    float menuW = 120;
    DrawRectangle(0, 32, menuW, h - 32, LUI::currentTheme->panelBg);

    for (int i = 0; i < tabCount; i++) {
      Rectangle tabRect = {5, 45.0f + i * 40, menuW - 10, 35};
      bool selected = (currentTab == i);

      if (selected) {
        DrawRectangleRounded(tabRect, 0.3f, 8, LUI::currentTheme->accent);
      } else if (CheckCollisionPointRec(mouse, tabRect)) {
        DrawRectangleRounded(tabRect, 0.3f, 8,
                             Fade(LUI::currentTheme->accent, 0.3f));
      }

      DrawText(tabs[i], tabRect.x + 10, tabRect.y + 10, 14,
               selected ? WHITE : LUI::currentTheme->text);

      if (CheckCollisionPointRec(mouse, tabRect) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentTab = i;
      }
    }

    // İçerik alanı
    float contentX = menuW + 20;
    float contentW = w - menuW - 30;
    float y = 50;

    // TAB 0: Görünüm
    if (currentTab == 0) {
      DrawText("Tema", contentX, y, 18, LUI::currentTheme->text);
      y += 35;

      Rectangle darkBtn = {contentX, y, 100, 35};
      Rectangle lightBtn = {contentX + 110, y, 100, 35};

      if (LUI::Button(darkBtn, "Koyu", !LUI::isDarkTheme)) {
        LUI::SetTheme(true);
      }
      if (LUI::Button(lightBtn, "Acik", LUI::isDarkTheme)) {
        LUI::SetTheme(false);
      }
      y += 60;

      DrawText("Accent Renk", contentX, y, 18, LUI::currentTheme->text);
      y += 35;

      Color colors[] = {{0, 122, 255, 255},
                        {255, 45, 85, 255},
                        {52, 199, 89, 255},
                        {255, 149, 0, 255},
                        {175, 82, 222, 255}};
      for (int i = 0; i < 5; i++) {
        Rectangle colorBtn = {contentX + i * 45, y, 40, 40};
        DrawRectangleRounded(colorBtn, 0.3f, 8, colors[i]);
        if (CheckCollisionPointRec(mouse, colorBtn) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          LUI::currentTheme->accent = colors[i];
        }
      }
    }

    // TAB 1: Ses
    else if (currentTab == 1) {
      DrawText("Ses Ayarları", contentX, y, 18, LUI::currentTheme->text);
      y += 40;

      DrawText("Ana Ses", contentX, y, 14, LUI::currentTheme->text);
      y += 22;
      masterVolume =
          LUI::Slider({contentX, y, contentW - 60, 16}, masterVolume, 0, 1);
      DrawText(TextFormat("%d%%", (int)(masterVolume * 100)),
               contentX + contentW - 50, y, 14, LUI::currentTheme->text);
      y += 45;

      DrawText("Müzik", contentX, y, 14, LUI::currentTheme->text);
      y += 22;
      musicVolume =
          LUI::Slider({contentX, y, contentW - 60, 16}, musicVolume, 0, 1);
      DrawText(TextFormat("%d%%", (int)(musicVolume * 100)),
               contentX + contentW - 50, y, 14, LUI::currentTheme->text);
      y += 45;

      DrawText("Efektler", contentX, y, 14, LUI::currentTheme->text);
      y += 22;
      effectVolume =
          LUI::Slider({contentX, y, contentW - 60, 16}, effectVolume, 0, 1);
      DrawText(TextFormat("%d%%", (int)(effectVolume * 100)),
               contentX + contentW - 50, y, 14, LUI::currentTheme->text);
    }

    // TAB 2: Hakkında
    else if (currentTab == 2) {
      // Logo
      float cx = contentX + contentW / 2;
      DrawCircleGradient(cx, y + 35, 40, LUI::currentTheme->accent,
                         Fade(LUI::currentTheme->accent, 0.3f));
      DrawText("L", cx - 15, y + 12, 48, WHITE);
      y += 90;

      const char *title = "LumanovOS";
      int tw = MeasureText(title, 28);
      DrawText(title, cx - tw / 2, y, 28, LUI::currentTheme->text);
      y += 35;

      DrawText("Versiyon 1.0", cx - MeasureText("Versiyon 1.0", 14) / 2, y, 14,
               LUI::currentTheme->textDim);
      y += 40;

      LUI::Separator(contentX, y, contentW);
      y += 20;

      DrawText("Sistem Bilgileri", contentX, y, 16, LUI::currentTheme->text);
      y += 30;

      DrawText("İşlemci:", contentX, y, 13, LUI::currentTheme->textDim);
      DrawText(cpuInfo, contentX + 70, y, 12, LUI::currentTheme->text);
      y += 24;

      DrawText("Bellek:", contentX, y, 13, LUI::currentTheme->textDim);
      DrawText(memInfo, contentX + 70, y, 13, LUI::currentTheme->text);
      y += 24;

      DrawText("Disk:", contentX, y, 13, LUI::currentTheme->textDim);
      DrawText(diskInfo, contentX + 70, y, 13, LUI::currentTheme->text);
      y += 24;

      DrawText("Kernel:", contentX, y, 13, LUI::currentTheme->textDim);
      DrawText(kernelInfo, contentX + 70, y, 13, LUI::currentTheme->text);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
