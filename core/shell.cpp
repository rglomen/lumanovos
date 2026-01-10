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
LUI::DockItem dockItems[] = {
    {"Dosyalar", "📁", {0, 122, 255, 255}, false, 0},
    {"Ayarlar", "⚙", {142, 142, 147, 255}, false, 0},
    {"Terminal", ">_", {40, 40, 45, 255}, false, 0},
    {"Notepad", "📝", {255, 204, 0, 255}, false, 0},
    {"Tarayici", "🌐", {255, 87, 51, 255}, false, 0},
    {"Monitor", "📊", {76, 175, 80, 255}, false, 0},
};
const int dockItemCount = 6;

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

  Rectangle menuRect = {(float)w - 200, 32, 190, 140};
  DrawRectangleRounded(menuRect, 0.1f, 10, LUI::currentTheme->windowBg);
  DrawRectangleRoundedLines(menuRect, 0.1f, 10, LUI::currentTheme->border);

  float y = menuRect.y + 10;

  // Tema değiştir
  Rectangle themeBtn = {menuRect.x + 10, y, 170, 32};
  if (LUI::ListItem(themeBtn, LUI::isDarkTheme ? "Acik Tema" : "Koyu Tema",
                    false, "🎨")) {
    LUI::SetTheme(!LUI::isDarkTheme);
  }
  y += 36;

  LUI::Separator(menuRect.x + 10, y, 170);
  y += 10;

  // Yeniden başlat
  Rectangle restartBtn = {menuRect.x + 10, y, 170, 32};
  if (LUI::ListItem(restartBtn, "Yeniden Baslat", false, "🔄")) {
    system("echo 120820 | sudo -S reboot");
  }
  y += 36;

  // Kapat
  Rectangle shutdownBtn = {menuRect.x + 10, y, 170, 32};
  if (LUI::ListItem(shutdownBtn, "Sistemi Kapat", false, "⏻")) {
    system("echo 120820 | sudo -S poweroff");
  }

  // Menü dışına tıklanırsa kapat
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
      !CheckCollisionPointRec(mouse, menuRect)) {
    showSystemMenu = false;
  }
}

void LoadWallpaper() {
  // İlk bulunan wallpaper'ı yükle
  if (FileExists("assets/wallpapers/default.png")) {
    wallpaper = LoadTexture("assets/wallpapers/default.png");
  } else if (FileExists("assets/wallpapers/default.jpg")) {
    wallpaper = LoadTexture("assets/wallpapers/default.jpg");
  } else if (FileExists("images/1.jpg")) {
    wallpaper = LoadTexture("images/1.jpg");
  }
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
  // Pencere başlat
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(1280, 800, "LumanovOS Desktop");
  SetTargetFPS(60);

  // Tema ayarla
  LUI::SetTheme(true); // Koyu tema

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
      ClearBackground(LUI::currentTheme->background);
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

    EndDrawing();
  }

  // Temizlik
  if (wallpaper.id > 0)
    UnloadTexture(wallpaper);
  CloseWindow();

  return 0;
}
