#include "../libs/ui/ui.h"
#include <cstdlib>
#include <cstring>
#include <string>

std::string urlInput;

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(550, 400, "Web Tarayıcı");
  SetTargetFPS(60);
  LUI::SetTheme(true);

  char urlText[512] = "";
  bool urlFocused = true;

  const char *quickLinks[][2] = {
      {"Google", "https://www.google.com"},
      {"YouTube", "https://www.youtube.com"},
      {"GitHub", "https://www.github.com"},
      {"Wikipedia", "https://www.wikipedia.org"},
      {"Reddit", "https://www.reddit.com"},
      {"Twitter", "https://www.twitter.com"},
  };
  const int linkCount = 6;

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);

    LUI::WindowResult winResult =
        LUI::DrawWindowFrame({0, 0, (float)w, (float)h}, "Web Tarayıcı", true);
    if (winResult.closeClicked)
      break;

    float y = 50;

    // URL Bar
    DrawText("URL:", 15, y + 5, 14, LUI::currentTheme->text);

    Rectangle urlBar = {60, y, (float)w - 140, 30};
    if (LUI::TextBox(urlBar, urlText, sizeof(urlText), &urlFocused)) {
      // Enter basıldı
      std::string url = urlText;
      if (!url.empty()) {
        if (url.find("http") == std::string::npos) {
          url = "https://" + url;
        }
        std::string cmd = "firefox '" + url + "' &";
        system(cmd.c_str());
      }
    }

    if (LUI::Button({(float)w - 75, y, 60, 30}, "Git")) {
      std::string url = urlText;
      if (!url.empty()) {
        if (url.find("http") == std::string::npos) {
          url = "https://" + url;
        }
        std::string cmd = "firefox '" + url + "' &";
        system(cmd.c_str());
      }
    }

    y += 50;
    LUI::Separator(15, y, w - 30);
    y += 20;

    // Hızlı Erişim
    DrawText("Hızlı Erişim", 15, y, 18, LUI::currentTheme->text);
    y += 35;

    for (int i = 0; i < linkCount; i++) {
      int col = i % 2;
      int row = i / 2;
      Rectangle btn = {15.0f + col * 260, y + row * 45, 250, 40};

      if (LUI::Button(btn, quickLinks[i][0])) {
        std::string cmd = "firefox '" + std::string(quickLinks[i][1]) + "' &";
        system(cmd.c_str());
      }
    }

    y += 150;
    LUI::Separator(15, y, w - 30);
    y += 20;

    // Bilgi
    DrawText("LumanovOS Web Tarayıcı", 15, y, 16, LUI::currentTheme->text);
    y += 30;
    DrawText("URL girin ve 'Git' butonuna basın", 15, y, 13,
             LUI::currentTheme->textDim);
    y += 20;
    DrawText("veya hızlı erişim butonlarını kullanın.", 15, y, 13,
             LUI::currentTheme->textDim);
    y += 30;
    DrawText("* Harici Firefox tarayıcısı açılacaktır", 15, y, 12,
             LUI::currentTheme->textDim);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
