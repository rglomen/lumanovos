#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>

// Tema Renkleri
struct Theme {
  Color background;
  Color dockBg;
  Color topBar;
  Color text;
  Color textSecondary;
  Color accent;
  Color cardBg;
  const char *name;
};

Theme darkTheme = {(Color){15, 15, 20, 255},    // background
                   (Color){30, 30, 40, 220},    // dockBg
                   (Color){25, 25, 35, 200},    // topBar
                   (Color){255, 255, 255, 255}, // text
                   (Color){180, 180, 190, 255}, // textSecondary
                   (Color){0, 122, 255, 255},   // accent (Apple Blue)
                   (Color){40, 40, 50, 200},    // cardBg
                   "Koyu Tema"};

Theme lightTheme = {(Color){240, 240, 245, 255}, // background
                    (Color){255, 255, 255, 220}, // dockBg
                    (Color){250, 250, 252, 230}, // topBar
                    (Color){30, 30, 35, 255},    // text
                    (Color){100, 100, 110, 255}, // textSecondary
                    (Color){0, 122, 255, 255},   // accent
                    (Color){255, 255, 255, 230}, // cardBg
                    "Acik Tema"};

// Uygulama bilgisi
struct App {
  const char *name;
  const char *icon; // Emoji benzeri karakter
  const char *command;
  Color iconColor;
  Color iconBgStart;
  Color iconBgEnd;
  float hoverAnim;  // 0.0 - 1.0 arası hover animasyonu
  float bounceAnim; // Tıklama animasyonu
};

// Animasyon değerleri
float dockY = 0;
float targetDockY = 0;
float clockPulse = 0;
bool isDarkTheme = true;
Theme *currentTheme = &darkTheme;

// Duvar kağıdı
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

// Yumuşak geçiş (lerp)
float Lerp(float a, float b, float t) { return a + (b - a) * t; }

// Renk karıştırma
Color LerpColor(Color a, Color b, float t) {
  return (Color){(unsigned char)(a.r + (b.r - a.r) * t),
                 (unsigned char)(a.g + (b.g - a.g) * t),
                 (unsigned char)(a.b + (b.b - a.b) * t),
                 (unsigned char)(a.a + (b.a - a.a) * t)};
}

// Gradient dikdörtgen çiz
void DrawGradientRoundedRect(Rectangle rect, float roundness, Color top,
                             Color bottom) {
  DrawRectangleRounded(rect, roundness, 20, top);
  // Alt kısma hafif gradient efekti
  DrawRectangleGradientV(rect.x + 5, rect.y + rect.height / 2, rect.width - 10,
                         rect.height / 2 - 5, (Color){0, 0, 0, 0}, bottom);
}

// İkon çizimi (basit şekiller ile)
void DrawAppIcon(float x, float y, float size, App &app, bool hovered) {
  float scale = 1.0f + app.hoverAnim * 0.15f;
  float actualSize = size * scale;
  float offset = (actualSize - size) / 2;

  // Bounce animasyonu
  float bounceOffset = sinf(app.bounceAnim * 3.14159f) * 8.0f;

  Rectangle iconRect = {x - offset, y - offset - bounceOffset, actualSize,
                        actualSize};

  // Gradient arka plan
  DrawRectangleRounded(iconRect, 0.25f, 15, app.iconBgStart);

  // İkon sembolü
  int fontSize = (int)(actualSize * 0.5f);
  int textWidth = MeasureText(app.icon, fontSize);
  DrawText(app.icon, iconRect.x + (actualSize - textWidth) / 2,
           iconRect.y + (actualSize - fontSize) / 2, fontSize, WHITE);

  // Parlaklık efekti (hover)
  if (hovered) {
    DrawRectangleRounded(
        iconRect, 0.25f, 15,
        (Color){255, 255, 255, (unsigned char)(40 * app.hoverAnim)});
  }

  // Alt gölge
  DrawEllipse(x + size / 2, y + size + 8 - bounceOffset,
              size / 2 * (0.8f + app.hoverAnim * 0.2f), 4,
              (Color){0, 0, 0, (unsigned char)(50 - bounceOffset * 5)});
}

// Özel mouse imleci çiz
void DrawCustomCursor(int x, int y) {
  // Ana imleç (ok şekli)
  Vector2 points[3] = {{(float)x, (float)y},
                       {(float)x, (float)y + 18},
                       {(float)x + 12, (float)y + 12}};

  // Gölge
  DrawTriangle((Vector2){points[0].x + 2, points[0].y + 2},
               (Vector2){points[1].x + 2, points[1].y + 2},
               (Vector2){points[2].x + 2, points[2].y + 2},
               (Color){0, 0, 0, 100});

  // Ana imleç
  DrawTriangle(points[0], points[1], points[2], WHITE);
  DrawTriangleLines(points[0], points[1], points[2], currentTheme->accent);

  // Küçük nokta
  DrawCircle(x + 1, y + 1, 2, currentTheme->accent);
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Desktop");
  SetTargetFPS(60);
  HideCursor(); // Sistem imlecini gizle, kendi imlecimizi çizeceğiz

  // Uygulamalar
  std::vector<App> dockApps = {
      {"Dosyalar", "D", "./file_manager &", (Color){255, 255, 255, 255},
       (Color){0, 122, 255, 255}, (Color){0, 80, 200, 255}, 0, 0},
      {"Ayarlar", "A", "./settings &", (Color){255, 255, 255, 255},
       (Color){142, 142, 147, 255}, (Color){100, 100, 110, 255}, 0, 0},
      {"Terminal", ">_", "xterm -fg white -bg black &",
       (Color){255, 255, 255, 255}, (Color){40, 40, 45, 255},
       (Color){20, 20, 25, 255}, 0, 0},
      {"Tarayici", "W", "firefox &", (Color){255, 255, 255, 255},
       (Color){255, 149, 0, 255}, (Color){200, 120, 0, 255}, 0, 0},
      {"Muzik", "M", "echo 'Muzik...'", (Color){255, 255, 255, 255},
       (Color){255, 45, 85, 255}, (Color){200, 30, 60, 255}, 0, 0},
  };

  // Duvar kağıdı
  std::vector<std::string> wp = GetWallpapers();
  Texture2D wallpaper = {0};
  if (!wp.empty())
    wallpaper = LoadTexture(wp[0].c_str());

  // Ana döngü
  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    clockPulse += dt;

    // Saat
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char timeStr[10];
    char dateStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M", ltm);
    strftime(dateStr, sizeof(dateStr), "%d %b %Y", ltm);

    // Mouse pozisyonu
    Vector2 mouse = GetMousePosition();

    // Dock animasyonu
    float screenH = GetScreenHeight();
    bool mouseNearDock = mouse.y > screenH - 120;
    targetDockY = mouseNearDock ? 0 : 20;
    dockY = Lerp(dockY, targetDockY, dt * 8);

    // Tema değiştirme (T tuşu)
    if (IsKeyPressed(KEY_T)) {
      isDarkTheme = !isDarkTheme;
      currentTheme = isDarkTheme ? &darkTheme : &lightTheme;
    }

    // Uygulama hover ve tıklama animasyonları
    float dockWidth = dockApps.size() * 75 + 40;
    float dockX = GetScreenWidth() / 2.0f - dockWidth / 2;
    float dockYPos = screenH - 85 + dockY;

    for (int i = 0; i < dockApps.size(); i++) {
      Rectangle iconRect = {dockX + 25 + (i * 75), dockYPos + 12, 50, 50};
      bool hovered = CheckCollisionPointRec(mouse, iconRect);

      // Hover animasyonu
      if (hovered) {
        dockApps[i].hoverAnim = std::min(1.0f, dockApps[i].hoverAnim + dt * 6);
      } else {
        dockApps[i].hoverAnim = std::max(0.0f, dockApps[i].hoverAnim - dt * 4);
      }

      // Bounce animasyonu
      if (dockApps[i].bounceAnim > 0) {
        dockApps[i].bounceAnim -= dt * 2;
        if (dockApps[i].bounceAnim < 0)
          dockApps[i].bounceAnim = 0;
      }

      // Tıklama
      if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        dockApps[i].bounceAnim = 1.0f;
        system(dockApps[i].command);
      }
    }

    // --- RENDER ---
    BeginDrawing();

    // Arka plan
    if (wallpaper.id > 0) {
      DrawTexturePro(
          wallpaper,
          (Rectangle){0, 0, (float)wallpaper.width, (float)wallpaper.height},
          (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
          (Vector2){0, 0}, 0.0f, WHITE);
      // Hafif overlay
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                    (Color){currentTheme->background.r,
                            currentTheme->background.g,
                            currentTheme->background.b, 100});
    } else {
      ClearBackground(currentTheme->background);
    }

    // Üst Bar
    DrawRectangle(0, 0, GetScreenWidth(), 32, currentTheme->topBar);
    DrawRectangleGradientV(0, 32, GetScreenWidth(), 5, (Color){0, 0, 0, 30},
                           (Color){0, 0, 0, 0});

    // Sol - Sistem adı
    DrawText("LumanovOS", 15, 8, 16, currentTheme->text);

    // Ortada - Tarih
    int dateWidth = MeasureText(dateStr, 14);
    DrawText(dateStr, GetScreenWidth() / 2 - dateWidth / 2, 9, 14,
             currentTheme->textSecondary);

    // Sağ - Saat (nabız animasyonu)
    float pulse = (sinf(clockPulse * 3.14159f) + 1) / 2 * 0.3f + 0.7f;
    Color clockColor = {(unsigned char)(currentTheme->text.r * pulse),
                        (unsigned char)(currentTheme->text.g * pulse),
                        (unsigned char)(currentTheme->text.b * pulse), 255};
    DrawText(timeStr, GetScreenWidth() - 60, 8, 16, clockColor);

    // Tema göstergesi (sağ üst)
    DrawCircle(GetScreenWidth() - 90, 16, 6, currentTheme->accent);
    DrawText(isDarkTheme ? "D" : "L", GetScreenWidth() - 93, 10, 12,
             isDarkTheme ? WHITE : BLACK);

    // Dock
    Rectangle dockRect = {dockX, dockYPos, dockWidth, 75};
    DrawRectangleRounded(dockRect, 0.35f, 20, currentTheme->dockBg);
    DrawRectangleRoundedLines(dockRect, 0.35f, 20, 1,
                              (Color){255, 255, 255, 30});

    // Dock ikonları
    for (int i = 0; i < dockApps.size(); i++) {
      float iconX = dockX + 25 + (i * 75);
      float iconY = dockYPos + 12;
      Rectangle iconRect = {iconX, iconY, 50, 50};
      bool hovered = CheckCollisionPointRec(mouse, iconRect);

      DrawAppIcon(iconX, iconY, 50, dockApps[i], hovered);

      // İsim tooltip (hover durumunda)
      if (dockApps[i].hoverAnim > 0.5f) {
        int nameWidth = MeasureText(dockApps[i].name, 12);
        float tooltipX = iconX + 25 - nameWidth / 2;
        float tooltipY = dockYPos - 25;
        float alpha = (dockApps[i].hoverAnim - 0.5f) * 2;

        DrawRectangleRounded(
            (Rectangle){tooltipX - 8, tooltipY - 4, (float)nameWidth + 16, 22},
            0.5f, 10, (Color){0, 0, 0, (unsigned char)(180 * alpha)});
        DrawText(dockApps[i].name, tooltipX, tooltipY, 12,
                 (Color){255, 255, 255, (unsigned char)(255 * alpha)});
      }
    }

    // Hoşgeldin kartı (ilk açılışta)
    static float welcomeAlpha = 1.0f;
    static float welcomeTimer = 3.0f;
    if (welcomeTimer > 0) {
      welcomeTimer -= dt;
      if (welcomeTimer < 1)
        welcomeAlpha = welcomeTimer;

      if (welcomeAlpha > 0) {
        float cx = GetScreenWidth() / 2.0f;
        float cy = GetScreenHeight() / 2.0f - 50;

        DrawRectangleRounded(
            (Rectangle){cx - 200, cy - 60, 400, 120}, 0.15f, 20,
            (Color){currentTheme->cardBg.r, currentTheme->cardBg.g,
                    currentTheme->cardBg.b,
                    (unsigned char)(currentTheme->cardBg.a * welcomeAlpha)});

        const char *welcome = "LumanovOS'a Hosgeldiniz!";
        const char *subtitle = "Tema degistirmek icin 'T' tusuna basin";
        int wWidth = MeasureText(welcome, 24);
        int sWidth = MeasureText(subtitle, 14);

        DrawText(welcome, cx - wWidth / 2, cy - 25, 24,
                 (Color){currentTheme->text.r, currentTheme->text.g,
                         currentTheme->text.b,
                         (unsigned char)(255 * welcomeAlpha)});
        DrawText(subtitle, cx - sWidth / 2, cy + 15, 14,
                 (Color){currentTheme->textSecondary.r,
                         currentTheme->textSecondary.g,
                         currentTheme->textSecondary.b,
                         (unsigned char)(255 * welcomeAlpha)});
      }
    }

    // Özel mouse imleci (EN SON çizilmeli)
    DrawCustomCursor(mouse.x, mouse.y);

    EndDrawing();
  }

  if (wallpaper.id > 0)
    UnloadTexture(wallpaper);
  CloseWindow();
  return 0;
}
