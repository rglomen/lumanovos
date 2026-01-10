#ifndef LUI_H
#define LUI_H

#include "raylib.h"
#include <string>

namespace LUI {

// ============================================================================
// TEMA YAPISI
// ============================================================================
struct Theme {
  Color background;
  Color windowBg;
  Color windowHeader;
  Color windowHeaderActive;
  Color panelBg;
  Color dockBg;
  Color text;
  Color textDim;
  Color accent;
  Color closeBtn;
  Color minimizeBtn;
  Color maximizeBtn;
  Color border;
  Color inputBg;
  Color buttonBg;
  Color buttonHover;
  Color success;
  Color warning;
  Color error;
};

extern Theme darkTheme;
extern Theme lightTheme;
extern Theme *currentTheme;
extern bool isDarkTheme;

void SetTheme(bool dark);
void LoadThemeFromFile(const char *path);
void SaveThemeToFile(const char *path);

// ============================================================================
// YARDIMCI FONKSİYONLAR
// ============================================================================
float Lerp(float a, float b, float t);
float Clamp(float v, float min, float max);
Color ColorLerp(Color a, Color b, float t);

// ============================================================================
// UI BİLEŞENLERİ
// ============================================================================

// Buton - tıklandığında true döner
bool Button(Rectangle rect, const char *text, bool disabled = false);
bool IconButton(Rectangle rect, const char *icon,
                const char *tooltip = nullptr);

// Metin girişi
bool TextBox(Rectangle rect, char *text, int maxLen, bool *focused);

// Liste öğesi
bool ListItem(Rectangle rect, const char *text, bool selected,
              const char *icon = nullptr);

// Scroll alanı - içerik yüksekliğinden fazlaysa scroll gösterir
void BeginScrollArea(Rectangle rect, float *scroll, float contentHeight);
void EndScrollArea();

// Panel başlığı
void PanelHeader(Rectangle rect, const char *title);

// Progress bar
void ProgressBar(Rectangle rect, float progress, Color color = {0});

// Slider
float Slider(Rectangle rect, float value, float min, float max);

// Checkbox
bool Checkbox(Rectangle rect, const char *label, bool *checked);

// Toggle switch
bool Toggle(Rectangle rect, bool *value);

// Separator line
void Separator(float x, float y, float width);

// Tooltip
void ShowTooltip(const char *text, Vector2 pos);

// ============================================================================
// PENCERE DEKORASYONLARİ
// ============================================================================
struct WindowResult {
  bool closeClicked;
  bool minimizeClicked;
  bool maximizeClicked;
  bool isDragging;
  Vector2 dragDelta;
};

WindowResult DrawWindowFrame(Rectangle bounds, const char *title, bool active);

// ============================================================================
// DOCK VE PANEL
// ============================================================================
struct DockItem {
  const char *name;
  const char *icon;
  Color color;
  bool isRunning;
  float hoverAnim;
};

void DrawDock(DockItem *items, int count, int *clickedIndex);
void DrawTopPanel(const char *systemName, bool *menuClicked);

// ============================================================================
// DİYALOGLAR
// ============================================================================
enum DialogResult {
  DIALOG_NONE,
  DIALOG_OK,
  DIALOG_CANCEL,
  DIALOG_YES,
  DIALOG_NO
};

DialogResult MessageBox(const char *title, const char *message,
                        bool hasCancel = false);
DialogResult ConfirmDialog(const char *title, const char *message);

// ============================================================================
// ANİMASYON
// ============================================================================
float EaseOutQuad(float t);
float EaseInOutQuad(float t);
float EaseOutBack(float t);

} // namespace LUI

#endif // LUI_H
