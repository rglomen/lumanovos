#include "../libs/ui/ui.h"
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

std::string filePath;
std::vector<std::string> lines;
int cursorLine = 0;
int cursorCol = 0;
bool modified = false;
float scrollY = 0;

void NewFile() {
  lines.clear();
  lines.push_back("");
  filePath.clear();
  cursorLine = 0;
  cursorCol = 0;
  modified = false;
  scrollY = 0;
}

void LoadFile(const std::string &path) {
  lines.clear();
  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  }
  if (lines.empty())
    lines.push_back("");
  filePath = path;
  cursorLine = 0;
  cursorCol = 0;
  modified = false;
  scrollY = 0;
}

void SaveFile() {
  if (filePath.empty())
    return;
  std::ofstream file(filePath);
  if (file.is_open()) {
    for (size_t i = 0; i < lines.size(); i++) {
      file << lines[i];
      if (i < lines.size() - 1)
        file << "\n";
    }
    file.close();
    modified = false;
  }
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(700, 500, "Notepad");
  SetTargetFPS(60);
  LUI::SetTheme(true);

  NewFile();

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);

    // Pencere başlığı
    std::string title = "Notepad";
    if (!filePath.empty()) {
      size_t pos = filePath.find_last_of('/');
      title += " - " +
               (pos != std::string::npos ? filePath.substr(pos + 1) : filePath);
    }
    if (modified)
      title += " *";

    LUI::WindowResult winResult =
        LUI::DrawWindowFrame({0, 0, (float)w, (float)h}, title.c_str(), true);
    if (winResult.closeClicked)
      break;

    // Toolbar
    float toolY = 40;
    if (LUI::Button({10, toolY, 60, 25}, "Yeni"))
      NewFile();
    if (LUI::Button({80, toolY, 60, 25}, "Kaydet", filePath.empty()))
      SaveFile();

    DrawText(filePath.empty() ? "Yeni Dosya" : filePath.c_str(), 160, toolY + 5,
             12, LUI::currentTheme->textDim);

    // Metin alanı
    Rectangle textArea = {5, 75, (float)w - 10, (float)h - 100};
    DrawRectangleRec(textArea, LUI::isDarkTheme ? BLACK : WHITE);

    float lineH = 18;
    float lineNumW = 40;
    int visibleLines = (textArea.height - 10) / lineH;

    // Scroll
    int wheel = GetMouseWheelMove();
    scrollY -= wheel * 3;
    scrollY = LUI::Clamp(
        scrollY, 0, std::max(0.0f, (float)lines.size() - visibleLines + 1));

    // Satırları çiz
    for (int i = 0; i < visibleLines && (i + (int)scrollY) < (int)lines.size();
         i++) {
      int lineIdx = i + (int)scrollY;
      float y = textArea.y + 5 + i * lineH;

      // Satır numarası
      DrawText(TextFormat("%3d", lineIdx + 1), textArea.x + 5, y, 12,
               LUI::currentTheme->textDim);
      DrawLine(textArea.x + lineNumW, y, textArea.x + lineNumW, y + lineH,
               LUI::currentTheme->border);

      // Aktif satır vurgusu
      if (lineIdx == cursorLine) {
        DrawRectangle(textArea.x + lineNumW + 2, y,
                      textArea.width - lineNumW - 5, lineH,
                      Fade(LUI::currentTheme->accent, 0.1f));
      }

      // Metin
      DrawText(lines[lineIdx].c_str(), textArea.x + lineNumW + 5, y + 2, 13,
               LUI::isDarkTheme ? WHITE : BLACK);

      // İmleç
      if (lineIdx == cursorLine && (int)(GetTime() * 2) % 2 == 0) {
        int cursorX =
            textArea.x + lineNumW + 5 +
            MeasureText(lines[lineIdx].substr(0, cursorCol).c_str(), 13);
        DrawRectangle(cursorX, y + 2, 2, 14, LUI::currentTheme->accent);
      }
    }

    // Klavye girişi
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 126) {
        lines[cursorLine].insert(cursorCol, 1, (char)key);
        cursorCol++;
        modified = true;
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ENTER)) {
      std::string newLine = lines[cursorLine].substr(cursorCol);
      lines[cursorLine] = lines[cursorLine].substr(0, cursorCol);
      lines.insert(lines.begin() + cursorLine + 1, newLine);
      cursorLine++;
      cursorCol = 0;
      modified = true;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (cursorCol > 0) {
        lines[cursorLine].erase(cursorCol - 1, 1);
        cursorCol--;
        modified = true;
      } else if (cursorLine > 0) {
        cursorCol = lines[cursorLine - 1].length();
        lines[cursorLine - 1] += lines[cursorLine];
        lines.erase(lines.begin() + cursorLine);
        cursorLine--;
        modified = true;
      }
    }

    if (IsKeyPressed(KEY_LEFT) && cursorCol > 0)
      cursorCol--;
    if (IsKeyPressed(KEY_RIGHT) && cursorCol < (int)lines[cursorLine].length())
      cursorCol++;
    if (IsKeyPressed(KEY_UP) && cursorLine > 0) {
      cursorLine--;
      cursorCol = std::min(cursorCol, (int)lines[cursorLine].length());
    }
    if (IsKeyPressed(KEY_DOWN) && cursorLine < (int)lines.size() - 1) {
      cursorLine++;
      cursorCol = std::min(cursorCol, (int)lines[cursorLine].length());
    }

    // Ctrl+S
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S) &&
        !filePath.empty()) {
      SaveFile();
    }

    // Durum çubuğu
    DrawRectangle(0, h - 25, w, 25, LUI::currentTheme->panelBg);
    DrawText(TextFormat("Satır: %d  Sütun: %d  %s", cursorLine + 1,
                        cursorCol + 1, modified ? "[Değiştirildi]" : ""),
             10, h - 20, 12, LUI::currentTheme->textDim);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
