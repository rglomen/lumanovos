#include "../libs/ui/ui.h"
#include <cstdlib>
#include <cstring>

// ============================================================================
// UYGULAMA TİPLERİ
// ============================================================================
enum AppType {
  APP_FILES,
  APP_SETTINGS,
  APP_TERMINAL,
  APP_NOTEPAD,
  APP_BROWSER,
  APP_MONITOR
};

const char *appPaths[] = {
    "./apps/files/files",       "./apps/settings/settings",
    "./apps/terminal/terminal", "./apps/notepad/notepad",
    "./apps/browser/browser",   "./apps/monitor/monitor"};

// ============================================================================
// GLOBAL DURUM
// ============================================================================
bool showSystemMenu = false;
Texture2D wallpaper = {0};

// Dock iconları - basit ve net semboller
LUI::DockItem dockItems[] = {
    {"Dosyalar", "F", {0, 122, 255, 255}, false, 0},
    {"Ayarlar", "S", {142, 142, 147, 255}, false, 0},
    {"Terminal", ">", {40, 40, 45, 255}, false, 0},
    {"Notepad", "N", {255, 204, 0, 255}, false, 0},
    {"Tarayici", "W", {255, 87, 51, 255}, false, 0},
    {"Monitor", "M", {76, 175, 80, 255}, false, 0},
};
const int dockItemCount = 6;

// ============================================================================
// MOUSE İMLECİ
// ============================================================================
void DrawCustomCursor(Vector2 pos) {
  // Modern ok imleci - beyaz dolgu, siyah kenar
  DrawTriangle(pos, {pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, WHITE);
  DrawTriangle(pos, {pos.x + 1, pos.y + 1}, {pos.x + 12, pos.y + 12},
               Fade(BLACK, 0.3f));
  DrawLineEx(pos, {pos.x, pos.y + 18}, 1.5f, BLACK);
  DrawLineEx(pos, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
  DrawLineEx({pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
}

// ============================================================================
// FONKSİYONLAR
// ============================================================================
void LaunchApp(AppType app) {
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "%s &", appPaths[app]);
  system(cmd);
  dockItems[app].isRunning = true;
}

void DrawSystemMenu() {
  int w = GetScreenWidth();
  Vector2 mouse = GetMousePosition();

  Rectangle menuRect = {(float)w - 220, 32, 210, 200};
  DrawRectangleRounded(menuRect, 0.1f, 10, LUI::currentTheme->windowBg);
  DrawRectangleRoundedLines(menuRect, 0.1f, 10, LUI::currentTheme->border);

  float y = menuRect.y + 10;

  // Tema değiştir
  Rectangle themeBtn = {menuRect.x + 10, y, 190, 36};
  if (LUI::ListItem(themeBtn,
                    LUI::isDarkTheme ? "Acik Temaya Gec" : "Koyu Temaya Gec",
                    false, "T")) {
    LUI::SetTheme(!LUI::isDarkTheme);
  }
  y += 40;

  // Ayarları aç
  Rectangle settingsBtn = {menuRect.x + 10, y, 190, 36};
  if (LUI::ListItem(settingsBtn, "Tum Ayarlar", false, "S")) {
    LaunchApp(APP_SETTINGS);
    showSystemMenu = false;
  }
  y += 44;

  LUI::Separator(menuRect.x + 10, y, 190);
  y += 10;

  // Yeniden başlat
  Rectangle restartBtn = {menuRect.x + 10, y, 190, 36};
  if (LUI::ListItem(restartBtn, "Yeniden Baslat", false, "R")) {
    system("echo 120820 | sudo -S reboot");
  }
  y += 40;

  // Kapat
  Rectangle shutdownBtn = {menuRect.x + 10, y, 190, 36};
  if (LUI::ListItem(shutdownBtn, "Sistemi Kapat", false, "X")) {
    system("echo 120820 | sudo -S poweroff");
  }

  // Menü dışına tıklanırsa kapat
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
      !CheckCollisionPointRec(mouse, menuRect)) {
    showSystemMenu = false;
  }
}

void LoadWallpaper() {
  const char *paths[] = {
      "assets/wallpapers/default.png", "assets/wallpapers/default.jpg",
      "assets/wallpapers/1.jpg", "images/1.jpg", "images/default.jpg"};

  for (int i = 0; i < 5; i++) {
    if (FileExists(paths[i])) {
      wallpaper = LoadTexture(paths[i]);
      break;
    }
  }
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(1280, 800, "LumanovOS Desktop");
  SetTargetFPS(60);

  // Sistem mouse'unu gizle
  HideCursor();

  // Tema ayarla
  LUI::SetTheme(true);

  // Wallpaper yükle
  LoadWallpaper();

  // Ana döngü
  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();

    // Arkaplan
    if (wallpaper.id > 0) {
      DrawTexturePro(wallpaper,
                     {0, 0, (float)wallpaper.width, (float)wallpaper.height},
                     {0, 0, (float)w, (float)h}, {0, 0}, 0, WHITE);
    } else {
      // Gradient arkaplan
      for (int i = 0; i < h; i++) {
        float t = (float)i / h;
        Color c = {(unsigned char)(20 + t * 10), (unsigned char)(22 + t * 15),
                   (unsigned char)(30 + t * 20), 255};
        DrawLine(0, i, w, i, c);
      }
    }

    // Üst panel
    bool menuClicked = false;
    LUI::DrawTopPanel("LumanovOS", &menuClicked);
    if (menuClicked)
      showSystemMenu = !showSystemMenu;

    // Dock
    int clickedApp = -1;
    LUI::DrawDock(dockItems, dockItemCount, &clickedApp);
    if (clickedApp >= 0) {
      LaunchApp((AppType)clickedApp);
    }

    // Sistem menüsü
    if (showSystemMenu) {
      DrawSystemMenu();
    }

    // Özel mouse imleci (en son çiz)
    DrawCustomCursor(mouse);

    EndDrawing();
  }

  // Temizlik
  ShowCursor();
  if (wallpaper.id > 0)
    UnloadTexture(wallpaper);
  CloseWindow();

  return 0;
}
