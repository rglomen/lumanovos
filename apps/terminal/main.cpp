#include "libs/ui/ui.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

std::vector<std::string> outputLines;
std::string currentInput;
float scrollY = 0;

void ExecuteCommand(const std::string &cmd) {
  outputLines.push_back("$ " + cmd);

  if (cmd == "clear") {
    outputLines.clear();
    return;
  }

  if (cmd == "help") {
    outputLines.push_back("Kullanılabilir komutlar:");
    outputLines.push_back("  clear  - Ekranı temizle");
    outputLines.push_back("  help   - Yardım göster");
    outputLines.push_back("  exit   - Terminali kapat");
    outputLines.push_back("  Diğer komutlar sistem shell'inde çalıştırılır");
    return;
  }

  // Gerçek komutu çalıştır
  FILE *fp = popen((cmd + " 2>&1").c_str(), "r");
  if (fp) {
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), fp)) {
      buffer[strcspn(buffer, "\n")] = 0;
      outputLines.push_back(buffer);
    }
    pclose(fp);
  } else {
    outputLines.push_back("Komut çalıştırılamadı");
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
  InitWindow(700, 450, "Terminal");
  SetTargetFPS(60);
  HideCursor();
  LUI::SetTheme(true);

  outputLines.push_back("LumanovOS Terminal v1.0");
  outputLines.push_back("Yardim icin 'help' yazin.");
  outputLines.push_back("");

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();
    ClearBackground({15, 15, 20, 255}); // Siyah terminal arka planı

    // Pencere çerçevesi
    LUI::WindowResult winResult =
        LUI::DrawWindowFrame({0, 0, (float)w, (float)h}, "Terminal", true);
    if (winResult.closeClicked)
      break;

    // Terminal alanı
    Rectangle termArea = {5, 40, (float)w - 10, (float)h - 75};
    DrawRectangleRec(termArea, BLACK);

    // Çıktı satırları
    float lineH = 18;
    int visibleLines = (termArea.height - 10) / lineH;

    // Scroll
    int wheel = GetMouseWheelMove();
    scrollY -= wheel * 3;
    int maxScroll = std::max(0, (int)outputLines.size() - visibleLines);
    scrollY = LUI::Clamp(scrollY, 0, maxScroll);

    for (int i = 0;
         i < visibleLines && (i + (int)scrollY) < (int)outputLines.size();
         i++) {
      int idx = i + (int)scrollY;
      const std::string &line = outputLines[idx];

      Color lineColor = GREEN;
      if (line.length() > 0 && line[0] == '$')
        lineColor = {100, 255, 100, 255};

      DrawText(line.c_str(), termArea.x + 10, termArea.y + 5 + i * lineH, 14,
               lineColor);
    }

    // Giriş satırı
    Rectangle inputArea = {5, (float)h - 35, (float)w - 10, 30};
    DrawRectangleRec(inputArea, {25, 25, 30, 255});

    DrawText("$ ", inputArea.x + 10, inputArea.y + 8, 14, GREEN);
    DrawText(currentInput.c_str(), inputArea.x + 30, inputArea.y + 8, 14,
             WHITE);

    // İmleç
    if ((int)(GetTime() * 2) % 2 == 0) {
      int cursorX = inputArea.x + 30 + MeasureText(currentInput.c_str(), 14);
      DrawRectangle(cursorX, inputArea.y + 6, 8, 18, GREEN);
    }

    // Klavye girişi
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 126) {
        currentInput += (char)key;
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !currentInput.empty()) {
      currentInput.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER)) {
      if (currentInput == "exit")
        break;
      if (!currentInput.empty()) {
        ExecuteCommand(currentInput);
        currentInput.clear();
        scrollY = std::max(0, (int)outputLines.size() - visibleLines);
      }
    }

    // Mouse imleci
    DrawCustomCursor(mouse);

    EndDrawing();
  }

  ShowCursor();
  CloseWindow();
  return 0;
}
