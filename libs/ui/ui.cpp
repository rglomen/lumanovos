#include "ui.h"
#include <cmath>
#include <cstring>
#include <ctime>

namespace LUI {

// ============================================================================
// TEMA TANIMLARI
// ============================================================================
Theme darkTheme = {
    {20, 22, 28, 255},    // background
    {35, 38, 48, 255},    // windowBg
    {45, 48, 58, 255},    // windowHeader
    {55, 58, 70, 255},    // windowHeaderActive
    {25, 27, 35, 245},    // panelBg
    {30, 32, 40, 230},    // dockBg
    {255, 255, 255, 255}, // text
    {140, 145, 160, 255}, // textDim
    {0, 122, 255, 255},   // accent
    {255, 95, 87, 255},   // closeBtn
    {255, 189, 46, 255},  // minimizeBtn
    {40, 200, 64, 255},   // maximizeBtn
    {60, 63, 75, 255},    // border
    {25, 27, 33, 255},    // inputBg
    {50, 53, 63, 255},    // buttonBg
    {60, 63, 75, 255},    // buttonHover
    {40, 200, 64, 255},   // success
    {255, 189, 46, 255},  // warning
    {255, 95, 87, 255},   // error
};

Theme lightTheme = {
    {240, 240, 245, 255}, // background
    {255, 255, 255, 255}, // windowBg
    {245, 245, 248, 255}, // windowHeader
    {235, 235, 240, 255}, // windowHeaderActive
    {250, 250, 252, 245}, // panelBg
    {255, 255, 255, 220}, // dockBg
    {30, 30, 35, 255},    // text
    {120, 120, 130, 255}, // textDim
    {0, 122, 255, 255},   // accent
    {255, 95, 87, 255},   // closeBtn
    {255, 189, 46, 255},  // minimizeBtn
    {40, 200, 64, 255},   // maximizeBtn
    {220, 220, 225, 255}, // border
    {245, 245, 248, 255}, // inputBg
    {230, 232, 236, 255}, // buttonBg
    {220, 222, 228, 255}, // buttonHover
    {40, 200, 64, 255},   // success
    {255, 189, 46, 255},  // warning
    {255, 95, 87, 255},   // error
};

Theme *currentTheme = &darkTheme;
bool isDarkTheme = true;

void SetTheme(bool dark) {
  isDarkTheme = dark;
  currentTheme = dark ? &darkTheme : &lightTheme;
}

// ============================================================================
// YARDIMCI FONKSİYONLAR
// ============================================================================
float Lerp(float a, float b, float t) { return a + (b - a) * t; }

float Clamp(float v, float min, float max) {
  return v < min ? min : (v > max ? max : v);
}

Color ColorLerp(Color a, Color b, float t) {
  return {
      (unsigned char)(a.r + (b.r - a.r) * t),
      (unsigned char)(a.g + (b.g - a.g) * t),
      (unsigned char)(a.b + (b.b - a.b) * t),
      (unsigned char)(a.a + (b.a - a.a) * t),
  };
}

float EaseOutQuad(float t) { return 1 - (1 - t) * (1 - t); }
float EaseInOutQuad(float t) {
  return t < 0.5f ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
}
float EaseOutBack(float t) {
  float c1 = 1.70158f;
  float c3 = c1 + 1;
  return 1 + c3 * pow(t - 1, 3) + c1 * pow(t - 1, 2);
}

// ============================================================================
// UI BİLEŞENLERİ
// ============================================================================
bool Button(Rectangle rect, const char *text, bool disabled) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect) && !disabled;
  bool pressed = hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  bool clicked = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

  Color bgColor = disabled  ? currentTheme->border
                  : pressed ? currentTheme->accent
                  : hover   ? currentTheme->buttonHover
                            : currentTheme->buttonBg;

  DrawRectangleRounded(rect, 0.3f, 8, bgColor);

  int textW = MeasureText(text, 14);
  Color textColor = disabled             ? currentTheme->textDim
                    : (pressed || hover) ? WHITE
                                         : currentTheme->text;
  DrawText(text, rect.x + (rect.width - textW) / 2,
           rect.y + (rect.height - 14) / 2, 14, textColor);

  return clicked && !disabled;
}

bool IconButton(Rectangle rect, const char *icon, const char *tooltip) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool pressed = hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  bool clicked = hover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

  Color bgColor = pressed ? currentTheme->accent
                  : hover ? Fade(currentTheme->accent, 0.3f)
                          : BLANK;

  DrawRectangleRounded(rect, 0.3f, 8, bgColor);

  int iconW = MeasureText(icon, 18);
  DrawText(icon, rect.x + (rect.width - iconW) / 2,
           rect.y + (rect.height - 18) / 2, 18,
           hover ? WHITE : currentTheme->text);

  if (hover && tooltip) {
    ShowTooltip(tooltip, {mouse.x + 15, mouse.y + 15});
  }

  return clicked;
}

bool TextBox(Rectangle rect, char *text, int maxLen, bool *focused) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect);

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    *focused = hover;
  }

  Color borderColor = *focused ? currentTheme->accent : currentTheme->border;
  DrawRectangleRounded(rect, 0.2f, 8, currentTheme->inputBg);
  DrawRectangleRoundedLines(rect, 0.2f, 8, borderColor);

  // Text
  DrawText(text, rect.x + 10, rect.y + (rect.height - 14) / 2, 14,
           currentTheme->text);

  // Cursor
  if (*focused && ((int)(GetTime() * 2) % 2 == 0)) {
    int textW = MeasureText(text, 14);
    DrawRectangle(rect.x + 10 + textW + 2, rect.y + (rect.height - 16) / 2, 2,
                  16, currentTheme->accent);
  }

  // Input handling
  if (*focused) {
    int key = GetCharPressed();
    while (key > 0) {
      int len = strlen(text);
      if (key >= 32 && key <= 126 && len < maxLen - 1) {
        text[len] = (char)key;
        text[len + 1] = '\0';
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      int len = strlen(text);
      if (len > 0)
        text[len - 1] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER)) {
      *focused = false;
      return true;
    }
  }

  return false;
}

bool ListItem(Rectangle rect, const char *text, bool selected,
              const char *icon) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (selected) {
    DrawRectangleRounded(rect, 0.2f, 8, Fade(currentTheme->accent, 0.3f));
  } else if (hover) {
    DrawRectangleRounded(rect, 0.2f, 8, Fade(currentTheme->accent, 0.15f));
  }

  float textX = rect.x + 10;
  if (icon) {
    DrawText(icon, rect.x + 10, rect.y + (rect.height - 16) / 2, 16,
             currentTheme->textDim);
    textX += 25;
  }

  DrawText(text, textX, rect.y + (rect.height - 14) / 2, 14,
           currentTheme->text);

  return clicked;
}

void ProgressBar(Rectangle rect, float progress, Color color) {
  if (color.a == 0)
    color = currentTheme->accent;
  progress = Clamp(progress, 0, 1);

  DrawRectangleRounded(rect, 0.4f, 8, currentTheme->border);
  if (progress > 0) {
    Rectangle fill = {rect.x, rect.y, rect.width * progress, rect.height};
    DrawRectangleRounded(fill, 0.4f, 8, color);
  }
}

float Slider(Rectangle rect, float value, float min, float max) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect);

  if (hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    value = min + (mouse.x - rect.x) / rect.width * (max - min);
    value = Clamp(value, min, max);
  }

  float progress = (value - min) / (max - min);
  DrawRectangleRounded(rect, 0.4f, 8, currentTheme->border);

  Rectangle fill = {rect.x, rect.y, rect.width * progress, rect.height};
  DrawRectangleRounded(fill, 0.4f, 8, currentTheme->accent);

  // Knob
  float knobX = rect.x + rect.width * progress;
  DrawCircle(knobX, rect.y + rect.height / 2, rect.height / 2 + 3,
             currentTheme->accent);
  DrawCircle(knobX, rect.y + rect.height / 2, rect.height / 2, WHITE);

  return value;
}

bool Checkbox(Rectangle rect, const char *label, bool *checked) {
  Vector2 mouse = GetMousePosition();
  Rectangle box = {rect.x, rect.y + (rect.height - 20) / 2, 20, 20};
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (clicked)
    *checked = !(*checked);

  DrawRectangleRounded(box, 0.2f, 8,
                       *checked ? currentTheme->accent : currentTheme->border);
  if (*checked) {
    DrawText("✓", box.x + 4, box.y + 2, 16, WHITE);
  }

  DrawText(label, rect.x + 28, rect.y + (rect.height - 14) / 2, 14,
           currentTheme->text);

  return clicked;
}

bool Toggle(Rectangle rect, bool *value) {
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (clicked)
    *value = !(*value);

  Color bgColor = *value ? currentTheme->accent : currentTheme->border;
  DrawRectangleRounded(rect, 0.5f, 16, bgColor);

  float knobX = *value ? rect.x + rect.width - rect.height + 4 : rect.x + 4;
  DrawCircle(knobX + (rect.height - 8) / 2, rect.y + rect.height / 2,
             (rect.height - 8) / 2, WHITE);

  return clicked;
}

void Separator(float x, float y, float width) {
  DrawRectangle(x, y, width, 1, currentTheme->border);
}

void ShowTooltip(const char *text, Vector2 pos) {
  int textW = MeasureText(text, 12);
  Rectangle bg = {pos.x, pos.y, (float)textW + 16, 24};

  DrawRectangleRounded(bg, 0.3f, 8, currentTheme->windowBg);
  DrawRectangleRoundedLines(bg, 0.3f, 8, currentTheme->border);
  DrawText(text, pos.x + 8, pos.y + 5, 12, currentTheme->text);
}

// ============================================================================
// PENCERE DEKORASYONLARİ
// ============================================================================
WindowResult DrawWindowFrame(Rectangle bounds, const char *title, bool active) {
  WindowResult result = {false, false, false, false, {0, 0}};
  Vector2 mouse = GetMousePosition();
  float headerH = 32;

  // Window shadow
  for (int i = 8; i > 0; i--) {
    DrawRectangleRounded({bounds.x - i, bounds.y - i, bounds.width + i * 2,
                          bounds.height + i * 2},
                         0.02f, 10, Fade(BLACK, 0.03f * (9 - i)));
  }

  // Window background
  DrawRectangleRounded(bounds, 0.02f, 10, currentTheme->windowBg);

  // Header
  Rectangle header = {bounds.x, bounds.y, bounds.width, headerH};
  DrawRectangleRounded(header, 0.02f, 10,
                       active ? currentTheme->windowHeaderActive
                              : currentTheme->windowHeader);

  // Title
  DrawText(title, bounds.x + 45, bounds.y + 9, 14, currentTheme->text);

  // Window buttons (macOS style)
  float btnY = bounds.y + 10;
  float btnSize = 12;

  Rectangle closeBtn = {bounds.x + 12, btnY, btnSize, btnSize};
  Rectangle minBtn = {bounds.x + 28, btnY, btnSize, btnSize};
  Rectangle maxBtn = {bounds.x + 44, btnY - 12, btnSize + 20, btnSize + 24};

  bool closeHover = CheckCollisionPointRec(mouse, closeBtn);
  bool minHover = CheckCollisionPointRec(mouse, minBtn);
  bool maxHover =
      CheckCollisionPointRec(mouse, {bounds.x + 44, btnY, btnSize, btnSize});

  DrawCircle(closeBtn.x + 6, closeBtn.y + 6, 6,
             closeHover ? currentTheme->closeBtn
                        : Fade(currentTheme->closeBtn, 0.7f));
  DrawCircle(minBtn.x + 6, minBtn.y + 6, 6,
             minHover ? currentTheme->minimizeBtn
                      : Fade(currentTheme->minimizeBtn, 0.7f));
  DrawCircle(bounds.x + 50, btnY + 6, 6,
             maxHover ? currentTheme->maximizeBtn
                      : Fade(currentTheme->maximizeBtn, 0.7f));

  // Button clicks
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (closeHover)
      result.closeClicked = true;
    if (minHover)
      result.minimizeClicked = true;
    if (maxHover)
      result.maximizeClicked = true;
  }

  // Dragging
  bool headerHover = CheckCollisionPointRec(mouse, header) && !closeHover &&
                     !minHover && !maxHover;
  if (headerHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    result.isDragging = true;
  }

  return result;
}

// ============================================================================
// DOCK
// ============================================================================
void DrawDock(DockItem *items, int count, int *clickedIndex) {
  *clickedIndex = -1;
  int w = GetScreenWidth();
  int h = GetScreenHeight();
  Vector2 mouse = GetMousePosition();
  float dt = GetFrameTime();

  float dockW = count * 60 + 30;
  float dockH = 60;
  float dockX = (w - dockW) / 2;
  float dockY = h - dockH - 10;

  // Dock background
  Rectangle dockRect = {dockX, dockY, dockW, dockH};
  DrawRectangleRounded(dockRect, 0.35f, 20, currentTheme->dockBg);
  DrawRectangleRoundedLines(dockRect, 0.35f, 20, Fade(WHITE, 0.1f));

  // Icons
  for (int i = 0; i < count; i++) {
    float iconX = dockX + 15 + i * 60;
    float iconY = dockY + 8;
    Rectangle iconRect = {iconX, iconY, 44, 44};

    bool hover = CheckCollisionPointRec(mouse, iconRect);

    // Animation
    float targetAnim = hover ? 1.0f : 0.0f;
    items[i].hoverAnim = Lerp(items[i].hoverAnim, targetAnim, dt * 10);

    float scale = 1.0f + items[i].hoverAnim * 0.15f;
    float offsetY = items[i].hoverAnim * -8;

    Rectangle scaledRect = {iconX + 22 - 22 * scale,
                            iconY + 22 - 22 * scale + offsetY, 44 * scale,
                            44 * scale};

    // Icon background
    DrawRectangleRounded(scaledRect, 0.25f, 8, items[i].color);

    // Icon text
    int iconW = MeasureText(items[i].icon, 22);
    DrawText(items[i].icon, scaledRect.x + (scaledRect.width - iconW) / 2,
             scaledRect.y + (scaledRect.height - 22) / 2, 22, WHITE);

    // Running indicator
    if (items[i].isRunning) {
      DrawCircle(iconX + 22, dockY + dockH - 5, 3, currentTheme->accent);
    }

    // Tooltip
    if (hover) {
      ShowTooltip(items[i].name, {iconX + 10, dockY - 30});
    }

    // Click
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      *clickedIndex = i;
    }
  }
}

// ============================================================================
// ÜST PANEL
// ============================================================================
void DrawTopPanel(const char *systemName, bool *menuClicked) {
  *menuClicked = false;
  int w = GetScreenWidth();
  Vector2 mouse = GetMousePosition();

  Rectangle panel = {0, 0, (float)w, 28};
  DrawRectangleRec(panel, currentTheme->panelBg);

  // System name
  DrawText(systemName, 12, 6, 14, currentTheme->text);

  // Clock
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  const char *timeStr = TextFormat("%02d:%02d", t->tm_hour, t->tm_min);
  int timeW = MeasureText(timeStr, 14);
  DrawText(timeStr, (w - timeW) / 2, 6, 14, currentTheme->text);

  // System menu button
  Rectangle menuBtn = {(float)w - 40, 2, 36, 24};
  bool menuHover = CheckCollisionPointRec(mouse, menuBtn);

  if (menuHover) {
    DrawRectangleRounded(menuBtn, 0.3f, 8, Fade(currentTheme->accent, 0.3f));
  }
  DrawText("☰", w - 32, 5, 16, currentTheme->text);

  if (menuHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    *menuClicked = true;
  }
}

} // namespace LUI
