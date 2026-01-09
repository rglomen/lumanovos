#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <time.h>
#include <vector>

// ============================================================================
// TEMA SİSTEMİ
// ============================================================================
struct Theme {
  Color background;
  Color windowBg;
  Color windowHeader;
  Color windowHeaderActive;
  Color dockBg;
  Color topBar;
  Color text;
  Color textDim;
  Color accent;
  Color closeBtn;
  Color minimizeBtn;
  Color maximizeBtn;
  Color border;
};

Theme darkTheme = {
    {20, 22, 28, 255},    // background
    {35, 38, 48, 255},    // windowBg
    {45, 48, 58, 255},    // windowHeader
    {55, 58, 70, 255},    // windowHeaderActive
    {30, 32, 40, 230},    // dockBg
    {25, 27, 35, 240},    // topBar
    {255, 255, 255, 255}, // text
    {140, 145, 160, 255}, // textDim
    {0, 122, 255, 255},   // accent
    {255, 95, 87, 255},   // closeBtn
    {255, 189, 46, 255},  // minimizeBtn
    {40, 200, 64, 255},   // maximizeBtn
    {60, 63, 75, 255},    // border
};

Theme lightTheme = {
    {240, 240, 245, 255}, // background
    {255, 255, 255, 255}, // windowBg
    {245, 245, 248, 255}, // windowHeader
    {235, 235, 240, 255}, // windowHeaderActive
    {255, 255, 255, 220}, // dockBg
    {250, 250, 252, 245}, // topBar
    {30, 30, 35, 255},    // text
    {120, 120, 130, 255}, // textDim
    {0, 122, 255, 255},   // accent
    {255, 95, 87, 255},   // closeBtn
    {255, 189, 46, 255},  // minimizeBtn
    {40, 200, 64, 255},   // maximizeBtn
    {220, 220, 225, 255}, // border
};

Theme *theme = &darkTheme;
bool isDarkTheme = true;

// ============================================================================
// PENCERE SİSTEMİ
// ============================================================================
enum WindowState { WIN_NORMAL, WIN_MINIMIZED, WIN_MAXIMIZED };
enum AppType {
  APP_NONE,
  APP_FILES,
  APP_SETTINGS,
  APP_TERMINAL,
  APP_UPDATER,
  APP_NOTEPAD,
  APP_BROWSER
};

struct Window {
  int id;
  std::string title;
  Rectangle bounds;
  Rectangle restoreBounds; // Maximize'dan dönmek için
  WindowState state;
  AppType appType;
  bool isActive;
  bool isDragging;
  bool isResizing;
  Vector2 dragOffset;
  float animProgress; // Animasyon için

  // Uygulama özel verileri
  std::string currentPath;
  std::vector<std::string> fileList;
  int scrollOffset;
  std::string terminalOutput;
  std::string terminalInput;

  // Notepad için
  std::string notepadFilePath;
  std::vector<std::string> notepadLines;
  int notepadCursorLine;
  int notepadCursorCol;
  bool notepadModified;
  int notepadScrollY;

  // Browser için
  std::string browserUrl;
  std::string browserContent;
  bool browserLoading;
};

std::vector<Window> windows;
int nextWindowId = 1;
int activeWindowId = -1;

// ============================================================================
// SİSTEM DURUMU
// ============================================================================
bool showSystemMenu = false;
bool showAppMenu = false;
float menuAnimProgress = 0;
Texture2D wallpaper = {0};

// Ayarlar için değişkenler
int settingsTab = 0; // 0: Görünüm, 1: Ekran, 2: Ses, 3: Hakkında
float masterVolume = 0.8f;
float musicVolume = 0.7f;
float effectVolume = 0.9f;
int currentWallpaperIndex = 0;
std::vector<std::string> wallpaperList;
std::string cpuInfo = "";
std::string memInfo = "";
std::string diskInfo = "";
std::string kernelInfo = "";

// Çözünürlük seçenekleri
struct Resolution {
  int width;
  int height;
  const char *name;
};
Resolution resolutions[] = {
    {800, 600, "800x600"},        {1024, 768, "1024x768"},
    {1280, 720, "1280x720 (HD)"}, {1280, 800, "1280x800"},
    {1366, 768, "1366x768"},      {1920, 1080, "1920x1080 (FHD)"},
};
int resolutionCount = 6;
int currentResolution = 3; // Varsayılan 1280x800

// Sistem bilgilerini çek
void LoadSystemInfo() {
  // CPU bilgisi
  FILE *fp = popen(
      "cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2", "r");
  if (fp) {
    char buf[256];
    if (fgets(buf, sizeof(buf), fp)) {
      cpuInfo = buf;
      cpuInfo.erase(cpuInfo.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(fp);
  }
  if (cpuInfo.empty())
    cpuInfo = "Bilinmiyor";

  // Bellek bilgisi
  fp = popen("free -h | grep Mem | awk '{print $2}'", "r");
  if (fp) {
    char buf[64];
    if (fgets(buf, sizeof(buf), fp)) {
      memInfo = buf;
      memInfo.erase(memInfo.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(fp);
  }
  if (memInfo.empty())
    memInfo = "Bilinmiyor";

  // Disk bilgisi
  fp = popen("df -h / | tail -1 | awk '{print $2}'", "r");
  if (fp) {
    char buf[64];
    if (fgets(buf, sizeof(buf), fp)) {
      diskInfo = buf;
      diskInfo.erase(diskInfo.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(fp);
  }
  if (diskInfo.empty())
    diskInfo = "Bilinmiyor";

  // Kernel bilgisi
  fp = popen("uname -r", "r");
  if (fp) {
    char buf[64];
    if (fgets(buf, sizeof(buf), fp)) {
      kernelInfo = buf;
      kernelInfo.erase(kernelInfo.find_last_not_of(" \n\r\t") + 1);
    }
    pclose(fp);
  }
  if (kernelInfo.empty())
    kernelInfo = "Bilinmiyor";
}

// Duvar kağıtlarını listele
void LoadWallpaperList() {
  wallpaperList.clear();
  DIR *dir = opendir("images");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name.find(".jpg") != std::string::npos ||
          name.find(".png") != std::string::npos) {
        wallpaperList.push_back("images/" + name);
      }
    }
    closedir(dir);
    std::sort(wallpaperList.begin(), wallpaperList.end());
  }
}

// ============================================================================
// YARDIMCI FONKSİYONLAR
// ============================================================================
float Lerp(float a, float b, float t) { return a + (b - a) * t; }
float Clamp(float v, float min, float max) {
  return v < min ? min : (v > max ? max : v);
}

std::vector<std::string> GetDirectoryContents(const std::string &path) {
  std::vector<std::string> items;
  DIR *dir = opendir(path.c_str());
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name != ".")
        items.push_back((ent->d_type == DT_DIR ? "[D] " : "[F] ") + name);
    }
    closedir(dir);
    std::sort(items.begin(), items.end());
  }
  return items;
}

// ============================================================================
// PENCERE YÖNETİMİ
// ============================================================================
Window *CreateWindow(const std::string &title, AppType app, float x, float y,
                     float w, float h) {
  Window win;
  win.id = nextWindowId++;
  win.title = title;
  win.bounds = {x, y, w, h};
  win.restoreBounds = win.bounds;
  win.state = WIN_NORMAL;
  win.appType = app;
  win.isActive = true;
  win.isDragging = false;
  win.isResizing = false;
  win.animProgress = 0;
  win.scrollOffset = 0;
  win.currentPath = ".";

  if (app == APP_FILES) {
    win.fileList = GetDirectoryContents(win.currentPath);
  }

  windows.push_back(win);
  activeWindowId = win.id;
  return &windows.back();
}

void CloseWindow(int id) {
  for (auto it = windows.begin(); it != windows.end(); ++it) {
    if (it->id == id) {
      windows.erase(it);
      if (activeWindowId == id) {
        activeWindowId = windows.empty() ? -1 : windows.back().id;
      }
      return;
    }
  }
}

void BringToFront(int id) {
  for (size_t i = 0; i < windows.size(); i++) {
    if (windows[i].id == id) {
      Window w = windows[i];
      windows.erase(windows.begin() + i);
      windows.push_back(w);
      activeWindowId = id;
      return;
    }
  }
}

Window *GetWindowById(int id) {
  for (auto &w : windows) {
    if (w.id == id)
      return &w;
  }
  return nullptr;
}

// ============================================================================
// PENCERE ÇİZİMİ
// ============================================================================
void DrawWindowFrame(Window &win, Vector2 mouse) {
  if (win.state == WIN_MINIMIZED)
    return;

  Rectangle bounds = win.bounds;
  bool isActive = (win.id == activeWindowId);
  float headerH = 32;

  // Pencere gölgesi
  for (int i = 0; i < 8; i++) {
    DrawRectangleRounded({bounds.x - i, bounds.y - i, bounds.width + i * 2,
                          bounds.height + i * 2},
                         0.02f, 10, Fade(BLACK, 0.03f * (8 - i)));
  }

  // Pencere arka planı
  DrawRectangleRounded(bounds, 0.02f, 10, theme->windowBg);

  // Başlık çubuğu
  Rectangle header = {bounds.x, bounds.y, bounds.width, headerH};
  DrawRectangleRounded(header, 0.02f, 10,
                       isActive ? theme->windowHeaderActive
                                : theme->windowHeader);

  // Pencere kontrol butonları (sol taraf - macOS tarzı)
  float btnY = bounds.y + headerH / 2;
  float btnX = bounds.x + 16;
  float btnR = 6;
  float btnSpacing = 20;

  // Kapat butonu
  Rectangle closeBtn = {btnX - btnR, btnY - btnR, btnR * 2, btnR * 2};
  bool closeHover = CheckCollisionPointRec(mouse, closeBtn);
  DrawCircle(btnX, btnY, btnR,
             closeHover ? theme->closeBtn : Fade(theme->closeBtn, 0.7f));
  if (closeHover)
    DrawText("x", btnX - 3, btnY - 5, 10, WHITE);

  // Küçült butonu
  btnX += btnSpacing;
  Rectangle minBtn = {btnX - btnR, btnY - btnR, btnR * 2, btnR * 2};
  bool minHover = CheckCollisionPointRec(mouse, minBtn);
  DrawCircle(btnX, btnY, btnR,
             minHover ? theme->minimizeBtn : Fade(theme->minimizeBtn, 0.7f));
  if (minHover)
    DrawText("-", btnX - 2, btnY - 6, 12, WHITE);

  // Büyüt butonu
  btnX += btnSpacing;
  Rectangle maxBtn = {btnX - btnR, btnY - btnR, btnR * 2, btnR * 2};
  bool maxHover = CheckCollisionPointRec(mouse, maxBtn);
  DrawCircle(btnX, btnY, btnR,
             maxHover ? theme->maximizeBtn : Fade(theme->maximizeBtn, 0.7f));
  if (maxHover)
    DrawText("+", btnX - 3, btnY - 5, 10, WHITE);

  // Başlık metni
  int titleWidth = MeasureText(win.title.c_str(), 14);
  DrawText(win.title.c_str(), bounds.x + bounds.width / 2 - titleWidth / 2,
           bounds.y + 9, 14, theme->text);

  // Boyutlandırma köşesi (sağ alt)
  if (win.state != WIN_MAXIMIZED) {
    Rectangle resizeCorner = {bounds.x + bounds.width - 15,
                              bounds.y + bounds.height - 15, 15, 15};
    bool resizeHover = CheckCollisionPointRec(mouse, resizeCorner);
    Color resizeColor = resizeHover ? theme->accent : theme->textDim;
    DrawTriangle({bounds.x + bounds.width, bounds.y + bounds.height},
                 {bounds.x + bounds.width - 12, bounds.y + bounds.height},
                 {bounds.x + bounds.width, bounds.y + bounds.height - 12},
                 resizeColor);
  }

  // İçerik alanı çerçevesi
  Rectangle content = {bounds.x + 1, bounds.y + headerH, bounds.width - 2,
                       bounds.height - headerH - 1};
  DrawRectangleRec(content, theme->windowBg);
}

// ============================================================================
// UYGULAMA İÇERİKLERİ
// ============================================================================
void DrawFileManager(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 5, win.bounds.y + 40,
                       win.bounds.width - 10, win.bounds.height - 50};

  // Yol çubuğu
  DrawRectangleRounded({content.x, content.y, content.width, 28}, 0.3f, 8,
                       theme->border);
  DrawText(win.currentPath.c_str(), content.x + 10, content.y + 7, 14,
           theme->text);

  // Dosya listesi
  float itemY = content.y + 35;
  float itemH = 28;
  int visibleItems = (content.height - 40) / itemH;

  for (int i = win.scrollOffset;
       i < std::min((int)win.fileList.size(), win.scrollOffset + visibleItems);
       i++) {
    Rectangle itemRect = {content.x, itemY, content.width, itemH};
    bool hover = CheckCollisionPointRec(mouse, itemRect);

    if (hover) {
      DrawRectangleRounded(itemRect, 0.2f, 8, Fade(theme->accent, 0.2f));

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        std::string item = win.fileList[i];
        if (item.substr(0, 3) == "[D]") {
          std::string folder = item.substr(4);
          if (folder == "..") {
            size_t pos = win.currentPath.find_last_of('/');
            if (pos != std::string::npos && pos > 0)
              win.currentPath = win.currentPath.substr(0, pos);
            else
              win.currentPath = ".";
          } else {
            win.currentPath += "/" + folder;
          }
          win.fileList = GetDirectoryContents(win.currentPath);
          win.scrollOffset = 0;
        }
      }
    }

    // İkon
    bool isDir = win.fileList[i].substr(0, 3) == "[D]";
    DrawText(isDir ? "D" : "F", content.x + 10, itemY + 6, 14,
             isDir ? theme->accent : theme->textDim);

    // Dosya adı
    std::string name = win.fileList[i].substr(4);
    DrawText(name.c_str(), content.x + 35, itemY + 6, 14, theme->text);

    itemY += itemH;
  }

  // Scroll
  if (CheckCollisionPointRec(mouse, content)) {
    int wheel = GetMouseWheelMove();
    win.scrollOffset -= wheel * 2;
    win.scrollOffset =
        Clamp(win.scrollOffset, 0,
              std::max(0, (int)win.fileList.size() - visibleItems));
  }
}

void DrawSettings(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 10, win.bounds.y + 45,
                       win.bounds.width - 20, win.bounds.height - 55};

  // Sol menü (tab seçimi)
  float menuW = 120;
  Rectangle menuArea = {content.x, content.y, menuW, content.height};
  DrawRectangleRounded(menuArea, 0.05f, 8, theme->border);

  const char *tabs[] = {"Gorunum", "Ekran", "Ses", "Hakkinda"};
  for (int i = 0; i < 4; i++) {
    Rectangle tabRect = {menuArea.x + 5, menuArea.y + 10 + i * 40, menuW - 10,
                         35};
    bool hover = CheckCollisionPointRec(mouse, tabRect);
    bool active = (settingsTab == i);

    if (active) {
      DrawRectangleRounded(tabRect, 0.3f, 8, theme->accent);
    } else if (hover) {
      DrawRectangleRounded(tabRect, 0.3f, 8, Fade(theme->accent, 0.3f));
    }

    DrawText(tabs[i], tabRect.x + 10, tabRect.y + 10, 14,
             active ? WHITE : theme->text);

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      settingsTab = i;
      if (i == 3 && cpuInfo.empty())
        LoadSystemInfo();
    }
  }

  // Sağ içerik alanı
  float contentX = content.x + menuW + 15;
  float contentW = content.width - menuW - 15;
  float y = content.y + 10;

  // TAB 0: GÖRÜNÜM
  if (settingsTab == 0) {
    DrawText("Tema", contentX, y, 18, theme->text);
    y += 30;

    Rectangle darkBtn = {contentX, y, 90, 35};
    Rectangle lightBtn = {contentX + 100, y, 90, 35};

    DrawRectangleRounded(darkBtn, 0.3f, 8,
                         isDarkTheme ? theme->accent : theme->border);
    DrawText("Koyu", darkBtn.x + 25, darkBtn.y + 10, 14,
             isDarkTheme ? WHITE : theme->text);

    DrawRectangleRounded(lightBtn, 0.3f, 8,
                         !isDarkTheme ? theme->accent : theme->border);
    DrawText("Acik", lightBtn.x + 28, lightBtn.y + 10, 14,
             !isDarkTheme ? WHITE : theme->text);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (CheckCollisionPointRec(mouse, darkBtn)) {
        isDarkTheme = true;
        theme = &darkTheme;
      } else if (CheckCollisionPointRec(mouse, lightBtn)) {
        isDarkTheme = false;
        theme = &lightTheme;
      }
    }

    y += 55;
    DrawText("Arka Plan", contentX, y, 18, theme->text);
    y += 30;

    // Duvar kağıdı listesini yükle
    if (wallpaperList.empty())
      LoadWallpaperList();

    for (int i = 0; i < std::min((int)wallpaperList.size(), 5); i++) {
      Rectangle wpBtn = {contentX, y, contentW - 10, 30};
      bool hover = CheckCollisionPointRec(mouse, wpBtn);
      bool active = (currentWallpaperIndex == i);

      if (active) {
        DrawRectangleRounded(wpBtn, 0.2f, 8, Fade(theme->accent, 0.4f));
      } else if (hover) {
        DrawRectangleRounded(wpBtn, 0.2f, 8, Fade(theme->accent, 0.2f));
      }

      // Dosya adını çıkar
      std::string name = wallpaperList[i];
      size_t pos = name.find_last_of('/');
      if (pos != std::string::npos)
        name = name.substr(pos + 1);

      DrawText(active ? "* " : "  ", contentX + 5, y + 7, 14, theme->accent);
      DrawText(name.c_str(), contentX + 25, y + 7, 14, theme->text);

      if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        currentWallpaperIndex = i;
        if (wallpaper.id > 0)
          UnloadTexture(wallpaper);
        wallpaper = LoadTexture(wallpaperList[i].c_str());
      }
      y += 35;
    }
  }

  // TAB 1: EKRAN
  else if (settingsTab == 1) {
    DrawText("Cozunurluk", contentX, y, 18, theme->text);
    y += 30;

    for (int i = 0; i < resolutionCount; i++) {
      Rectangle resBtn = {contentX, y, contentW - 10, 32};
      bool hover = CheckCollisionPointRec(mouse, resBtn);
      bool active = (currentResolution == i);

      if (active) {
        DrawRectangleRounded(resBtn, 0.2f, 8, Fade(theme->accent, 0.4f));
      } else if (hover) {
        DrawRectangleRounded(resBtn, 0.2f, 8, Fade(theme->accent, 0.2f));
      }

      DrawText(active ? "[*]" : "[ ]", contentX + 8, y + 8, 14, theme->accent);
      DrawText(resolutions[i].name, contentX + 40, y + 8, 14, theme->text);

      if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !active) {
        currentResolution = i;
        // Çözünürlük değiştir
        SetWindowSize(resolutions[i].width, resolutions[i].height);
      }
      y += 38;
    }

    y += 10;
    DrawText("Not: Degisiklik aninda uygulanir", contentX, y, 12,
             theme->textDim);
  }

  // TAB 2: SES
  else if (settingsTab == 2) {
    DrawText("Ses Ayarlari", contentX, y, 18, theme->text);
    y += 35;

    // Master Volume
    DrawText("Ana Ses", contentX, y, 14, theme->text);
    y += 22;
    Rectangle masterBar = {contentX, y, contentW - 60, 20};
    DrawRectangleRounded(masterBar, 0.4f, 8, theme->border);
    DrawRectangleRounded(
        {masterBar.x, masterBar.y, masterBar.width * masterVolume, 20}, 0.4f, 8,
        theme->accent);
    DrawText(TextFormat("%d%%", (int)(masterVolume * 100)),
             masterBar.x + masterBar.width + 10, y + 2, 14, theme->text);

    if (CheckCollisionPointRec(mouse, masterBar) &&
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      masterVolume = Clamp((mouse.x - masterBar.x) / masterBar.width, 0, 1);
    }
    y += 40;

    // Music Volume
    DrawText("Muzik", contentX, y, 14, theme->text);
    y += 22;
    Rectangle musicBar = {contentX, y, contentW - 60, 20};
    DrawRectangleRounded(musicBar, 0.4f, 8, theme->border);
    DrawRectangleRounded(
        {musicBar.x, musicBar.y, musicBar.width * musicVolume, 20}, 0.4f, 8,
        (Color){255, 149, 0, 255});
    DrawText(TextFormat("%d%%", (int)(musicVolume * 100)),
             musicBar.x + musicBar.width + 10, y + 2, 14, theme->text);

    if (CheckCollisionPointRec(mouse, musicBar) &&
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      musicVolume = Clamp((mouse.x - musicBar.x) / musicBar.width, 0, 1);
    }
    y += 40;

    // Effect Volume
    DrawText("Efektler", contentX, y, 14, theme->text);
    y += 22;
    Rectangle effectBar = {contentX, y, contentW - 60, 20};
    DrawRectangleRounded(effectBar, 0.4f, 8, theme->border);
    DrawRectangleRounded(
        {effectBar.x, effectBar.y, effectBar.width * effectVolume, 20}, 0.4f, 8,
        (Color){40, 200, 64, 255});
    DrawText(TextFormat("%d%%", (int)(effectVolume * 100)),
             effectBar.x + effectBar.width + 10, y + 2, 14, theme->text);

    if (CheckCollisionPointRec(mouse, effectBar) &&
        IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      effectVolume = Clamp((mouse.x - effectBar.x) / effectBar.width, 0, 1);
    }
  }

  // TAB 3: HAKKINDA
  else if (settingsTab == 3) {
    // LumanovOS Logo ve başlık
    float cx = contentX + (contentW - 10) / 2;

    DrawCircleGradient(cx, y + 30, 35, theme->accent,
                       Fade(theme->accent, 0.3f));
    DrawText("L", cx - 12, y + 10, 40, WHITE);
    y += 80;

    const char *title = "LumanovOS";
    int tw = MeasureText(title, 28);
    DrawText(title, cx - tw / 2, y, 28, theme->text);
    y += 35;

    DrawText("Versiyon 2.1", cx - MeasureText("Versiyon 2.1", 14) / 2, y, 14,
             theme->textDim);
    y += 30;

    // Ayırıcı çizgi
    DrawRectangle(contentX + 20, y, contentW - 50, 1, theme->border);
    y += 20;

    // Sistem bilgileri
    DrawText("Sistem Bilgileri", contentX, y, 16, theme->text);
    y += 28;

    DrawText("Islemci:", contentX + 10, y, 13, theme->textDim);
    DrawText(cpuInfo.c_str(), contentX + 80, y, 12, theme->text);
    y += 22;

    DrawText("Bellek:", contentX + 10, y, 13, theme->textDim);
    DrawText(memInfo.c_str(), contentX + 80, y, 13, theme->text);
    y += 22;

    DrawText("Disk:", contentX + 10, y, 13, theme->textDim);
    DrawText(diskInfo.c_str(), contentX + 80, y, 13, theme->text);
    y += 22;

    DrawText("Kernel:", contentX + 10, y, 13, theme->textDim);
    DrawText(kernelInfo.c_str(), contentX + 80, y, 13, theme->text);
    y += 30;

    // Ayırıcı çizgi
    DrawRectangle(contentX + 20, y, contentW - 50, 1, theme->border);
    y += 15;

    DrawText("Raylib 5.x Tabanli Masaustu Ortami", contentX + 10, y, 11,
             theme->textDim);
    y += 16;
    DrawText("2024-2025 Lumanov Projesi", contentX + 10, y, 11, theme->textDim);
  }
}

void DrawTerminal(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 5, win.bounds.y + 40,
                       win.bounds.width - 10, win.bounds.height - 50};

  // Arka plan
  DrawRectangleRounded(content, 0.02f, 8, BLACK);

  // Çıktı
  DrawText(win.terminalOutput.c_str(), content.x + 10, content.y + 10, 14,
           GREEN);

  // Prompt
  float promptY = content.y + content.height - 30;
  DrawText("$ ", content.x + 10, promptY, 14, GREEN);
  DrawText(win.terminalInput.c_str(), content.x + 25, promptY, 14, WHITE);

  // Yanıp sönen imleç
  if ((int)(GetTime() * 2) % 2 == 0) {
    DrawRectangle(content.x + 25 + MeasureText(win.terminalInput.c_str(), 14),
                  promptY, 8, 16, GREEN);
  }

  // Tuş girişi
  int key = GetCharPressed();
  while (key > 0) {
    if (key >= 32 && key <= 126) {
      win.terminalInput += (char)key;
    }
    key = GetCharPressed();
  }

  if (IsKeyPressed(KEY_BACKSPACE) && !win.terminalInput.empty()) {
    win.terminalInput.pop_back();
  }

  if (IsKeyPressed(KEY_ENTER) && !win.terminalInput.empty()) {
    win.terminalOutput += "$ " + win.terminalInput + "\n";
    if (win.terminalInput == "clear") {
      win.terminalOutput = "";
    } else if (win.terminalInput == "help") {
      win.terminalOutput += "Komutlar: help, clear, date, whoami\n";
    } else if (win.terminalInput == "date") {
      time_t now = time(0);
      win.terminalOutput += std::string(ctime(&now));
    } else if (win.terminalInput == "whoami") {
      win.terminalOutput += "lomen\n";
    } else {
      win.terminalOutput += "Komut bulunamadi: " + win.terminalInput + "\n";
    }
    win.terminalInput = "";
  }
}

void DrawUpdater(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 10, win.bounds.y + 45,
                       win.bounds.width - 20, win.bounds.height - 55};

  float cx = content.x + content.width / 2;
  float cy = content.y + 40;

  // Başlık
  const char *title = "Sistem Guncellemesi";
  DrawText(title, cx - MeasureText(title, 18) / 2, cy, 18, theme->text);

  cy += 50;

  // Butonlar
  Rectangle checkBtn = {cx - 100, cy, 200, 40};
  bool checkHover = CheckCollisionPointRec(mouse, checkBtn);
  DrawRectangleRounded(checkBtn, 0.3f, 10,
                       checkHover ? Fade(theme->accent, 0.8f) : theme->accent);
  DrawText("Guncelleme Kontrol Et", checkBtn.x + 20, checkBtn.y + 12, 14,
           WHITE);

  if (checkHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    // Launcher'a dön
    system("cd .. && ./lumanov_launcher &");
    exit(0);
  }

  cy += 60;

  Rectangle restartBtn = {cx - 100, cy, 200, 40};
  bool restartHover = CheckCollisionPointRec(mouse, restartBtn);
  DrawRectangleRounded(restartBtn, 0.3f, 10,
                       restartHover ? Fade(theme->minimizeBtn, 0.8f)
                                    : theme->minimizeBtn);
  DrawText("Sistemi Yeniden Baslat", restartBtn.x + 15, restartBtn.y + 12, 14,
           BLACK);

  if (restartHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    system("echo 120820 | sudo -S reboot");
  }

  cy += 60;

  Rectangle shutdownBtn = {cx - 100, cy, 200, 40};
  bool shutdownHover = CheckCollisionPointRec(mouse, shutdownBtn);
  DrawRectangleRounded(shutdownBtn, 0.3f, 10,
                       shutdownHover ? Fade(theme->closeBtn, 0.8f)
                                     : theme->closeBtn);
  DrawText("Sistemi Kapat", shutdownBtn.x + 45, shutdownBtn.y + 12, 14, WHITE);

  if (shutdownHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    system("echo 120820 | sudo -S poweroff");
  }
}

// ============================================================================
// NOTEPAD UYGULAMASI
// ============================================================================
void LoadTextFile(Window &win, const std::string &path) {
  win.notepadFilePath = path;
  win.notepadLines.clear();
  win.notepadModified = false;

  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      win.notepadLines.push_back(line);
    }
    file.close();
  }
  if (win.notepadLines.empty()) {
    win.notepadLines.push_back("");
  }
  win.notepadCursorLine = 0;
  win.notepadCursorCol = 0;
  win.notepadScrollY = 0;

  // Pencere başlığını güncelle
  size_t pos = path.find_last_of('/');
  std::string filename =
      (pos != std::string::npos) ? path.substr(pos + 1) : path;
  win.title = "Notepad - " + filename;
}

void SaveTextFile(Window &win) {
  if (win.notepadFilePath.empty())
    return;

  std::ofstream file(win.notepadFilePath);
  if (file.is_open()) {
    for (size_t i = 0; i < win.notepadLines.size(); i++) {
      file << win.notepadLines[i];
      if (i < win.notepadLines.size() - 1)
        file << "\n";
    }
    file.close();
    win.notepadModified = false;
  }
}

void DrawNotepad(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 5, win.bounds.y + 40,
                       win.bounds.width - 10, win.bounds.height - 50};

  // Toolbar
  float toolY = content.y;
  Rectangle newBtn = {content.x, toolY, 60, 25};
  Rectangle openBtn = {content.x + 65, toolY, 60, 25};
  Rectangle saveBtn = {content.x + 130, toolY, 60, 25};

  bool newHover = CheckCollisionPointRec(mouse, newBtn);
  bool openHover = CheckCollisionPointRec(mouse, openBtn);
  bool saveHover = CheckCollisionPointRec(mouse, saveBtn);

  DrawRectangleRounded(newBtn, 0.3f, 8,
                       newHover ? theme->accent : theme->border);
  DrawText("Yeni", newBtn.x + 12, newBtn.y + 5, 13,
           newHover ? WHITE : theme->text);

  DrawRectangleRounded(openBtn, 0.3f, 8,
                       openHover ? theme->accent : theme->border);
  DrawText("Ac", openBtn.x + 20, openBtn.y + 5, 13,
           openHover ? WHITE : theme->text);

  Color saveBtnColor =
      win.notepadModified ? (Color){255, 149, 0, 255} : theme->border;
  DrawRectangleRounded(saveBtn, 0.3f, 8,
                       saveHover ? theme->accent : saveBtnColor);
  DrawText("Kaydet", saveBtn.x + 5, saveBtn.y + 5, 13,
           saveHover ? WHITE : theme->text);

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (newHover) {
      win.notepadLines.clear();
      win.notepadLines.push_back("");
      win.notepadFilePath = "";
      win.notepadCursorLine = 0;
      win.notepadCursorCol = 0;
      win.notepadModified = false;
      win.title = "Notepad - Yeni Dosya";
    }
    if (saveHover && !win.notepadFilePath.empty()) {
      SaveTextFile(win);
    }
  }

  // Dosya yolu göstergesi
  DrawText(win.notepadFilePath.empty() ? "Yeni Dosya"
                                       : win.notepadFilePath.c_str(),
           content.x + 200, toolY + 6, 11, theme->textDim);

  // Metin alanı
  Rectangle textArea = {content.x, content.y + 30, content.width,
                        content.height - 35};
  DrawRectangleRounded(textArea, 0.02f, 8, isDarkTheme ? BLACK : WHITE);

  float lineH = 18;
  float lineNumW = 35;
  int visibleLines = (textArea.height - 10) / lineH;

  // Scroll kontrolü
  if (CheckCollisionPointRec(mouse, textArea)) {
    int wheel = GetMouseWheelMove();
    win.notepadScrollY -= wheel * 3;
    win.notepadScrollY =
        Clamp(win.notepadScrollY, 0,
              std::max(0, (int)win.notepadLines.size() - visibleLines + 1));
  }

  // Satırları çiz
  for (int i = 0; i < visibleLines &&
                  (i + win.notepadScrollY) < (int)win.notepadLines.size();
       i++) {
    int lineIdx = i + win.notepadScrollY;
    float y = textArea.y + 5 + i * lineH;

    // Satır numarası
    DrawText(TextFormat("%3d", lineIdx + 1), textArea.x + 3, y, 12,
             theme->textDim);

    // Ayırıcı çizgi
    DrawLine(textArea.x + lineNumW, y, textArea.x + lineNumW, y + lineH,
             theme->border);

    // Aktif satır vurgusu
    if (lineIdx == win.notepadCursorLine) {
      DrawRectangle(textArea.x + lineNumW + 2, y, textArea.width - lineNumW - 5,
                    lineH, Fade(theme->accent, 0.1f));
    }

    // Metin
    std::string &line = win.notepadLines[lineIdx];
    DrawText(line.c_str(), textArea.x + lineNumW + 5, y + 2, 13,
             isDarkTheme ? WHITE : BLACK);

    // İmleç
    if (lineIdx == win.notepadCursorLine && (int)(GetTime() * 2) % 2 == 0) {
      int cursorX =
          textArea.x + lineNumW + 5 +
          MeasureText(line.substr(0, win.notepadCursorCol).c_str(), 13);
      DrawRectangle(cursorX, y + 2, 2, 14, theme->accent);
    }
  }

  // Metin alanına tıklama - satır seçimi
  if (CheckCollisionPointRec(mouse, textArea) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    int clickedLine = (mouse.y - textArea.y - 5) / lineH + win.notepadScrollY;
    if (clickedLine >= 0 && clickedLine < (int)win.notepadLines.size()) {
      win.notepadCursorLine = clickedLine;
      // Sütun pozisyonunu hesapla (yaklaşık)
      win.notepadCursorCol =
          std::min((int)win.notepadLines[clickedLine].length(),
                   (int)((mouse.x - textArea.x - lineNumW - 5) / 7));
    }
  }

  // Klavye girişi
  if (win.id == activeWindowId) {
    // Karakter girişi
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 126) {
        std::string &line = win.notepadLines[win.notepadCursorLine];
        line.insert(win.notepadCursorCol, 1, (char)key);
        win.notepadCursorCol++;
        win.notepadModified = true;
      }
      key = GetCharPressed();
    }

    // Özel tuşlar
    if (IsKeyPressed(KEY_ENTER)) {
      std::string &line = win.notepadLines[win.notepadCursorLine];
      std::string newLine = line.substr(win.notepadCursorCol);
      line = line.substr(0, win.notepadCursorCol);
      win.notepadLines.insert(
          win.notepadLines.begin() + win.notepadCursorLine + 1, newLine);
      win.notepadCursorLine++;
      win.notepadCursorCol = 0;
      win.notepadModified = true;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (win.notepadCursorCol > 0) {
        std::string &line = win.notepadLines[win.notepadCursorLine];
        line.erase(win.notepadCursorCol - 1, 1);
        win.notepadCursorCol--;
        win.notepadModified = true;
      } else if (win.notepadCursorLine > 0) {
        std::string &prevLine = win.notepadLines[win.notepadCursorLine - 1];
        win.notepadCursorCol = prevLine.length();
        prevLine += win.notepadLines[win.notepadCursorLine];
        win.notepadLines.erase(win.notepadLines.begin() +
                               win.notepadCursorLine);
        win.notepadCursorLine--;
        win.notepadModified = true;
      }
    }

    if (IsKeyPressed(KEY_LEFT) && win.notepadCursorCol > 0)
      win.notepadCursorCol--;
    if (IsKeyPressed(KEY_RIGHT)) {
      if (win.notepadCursorCol <
          (int)win.notepadLines[win.notepadCursorLine].length())
        win.notepadCursorCol++;
    }
    if (IsKeyPressed(KEY_UP) && win.notepadCursorLine > 0) {
      win.notepadCursorLine--;
      win.notepadCursorCol =
          std::min(win.notepadCursorCol,
                   (int)win.notepadLines[win.notepadCursorLine].length());
    }
    if (IsKeyPressed(KEY_DOWN) &&
        win.notepadCursorLine < (int)win.notepadLines.size() - 1) {
      win.notepadCursorLine++;
      win.notepadCursorCol =
          std::min(win.notepadCursorCol,
                   (int)win.notepadLines[win.notepadCursorLine].length());
    }

    // Ctrl+S kaydet
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
      if (!win.notepadFilePath.empty())
        SaveTextFile(win);
    }
  }

  // Durum çubuğu
  DrawText(TextFormat("Satir: %d  Sutun: %d  %s", win.notepadCursorLine + 1,
                      win.notepadCursorCol + 1,
                      win.notepadModified ? "[Degistirildi]" : ""),
           textArea.x + 5, textArea.y + textArea.height - 18, 11,
           theme->textDim);
}

// ============================================================================
// BROWSER UYGULAMASI
// ============================================================================
void DrawBrowser(Window &win, Vector2 mouse) {
  Rectangle content = {win.bounds.x + 5, win.bounds.y + 40,
                       win.bounds.width - 10, win.bounds.height - 50};

  // URL Bar
  Rectangle urlBar = {content.x, content.y, content.width - 70, 30};
  Rectangle goBtn = {content.x + content.width - 65, content.y, 60, 30};

  DrawRectangleRounded(urlBar, 0.3f, 8, isDarkTheme ? BLACK : WHITE);
  DrawRectangleRoundedLines(urlBar, 0.3f, 8, theme->border);

  // URL metni
  DrawText(win.browserUrl.c_str(), urlBar.x + 10, urlBar.y + 8, 13,
           isDarkTheme ? WHITE : BLACK);

  // İmleç
  if (win.id == activeWindowId && (int)(GetTime() * 2) % 2 == 0) {
    int cursorX = urlBar.x + 10 + MeasureText(win.browserUrl.c_str(), 13);
    DrawRectangle(cursorX, urlBar.y + 6, 2, 18, theme->accent);
  }

  // Git butonu
  bool goHover = CheckCollisionPointRec(mouse, goBtn);
  DrawRectangleRounded(goBtn, 0.3f, 8, goHover ? theme->accent : theme->border);
  DrawText("Git", goBtn.x + 18, goBtn.y + 8, 13, goHover ? WHITE : theme->text);

  // Hızlı linkler
  float y = content.y + 40;
  DrawText("Hizli Erisim:", content.x, y, 14, theme->textDim);
  y += 25;

  const char *quickLinks[][2] = {
      {"Google", "https://www.google.com"},
      {"YouTube", "https://www.youtube.com"},
      {"GitHub", "https://www.github.com"},
      {"Wikipedia", "https://www.wikipedia.org"},
  };

  for (int i = 0; i < 4; i++) {
    Rectangle linkBtn = {content.x + (i % 2) * 120, y + (i / 2) * 35, 110, 30};
    bool hover = CheckCollisionPointRec(mouse, linkBtn);

    DrawRectangleRounded(linkBtn, 0.3f, 8,
                         hover ? Fade(theme->accent, 0.3f) : theme->border);
    DrawText(quickLinks[i][0], linkBtn.x + 10, linkBtn.y + 8, 13,
             hover ? theme->accent : theme->text);

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      std::string cmd = "firefox '" + std::string(quickLinks[i][1]) + "' &";
      system(cmd.c_str());
    }
  }

  y += 80;

  // Bilgi
  DrawRectangle(content.x, y, content.width, 1, theme->border);
  y += 15;

  DrawText("LumanovOS Web Tarayici", content.x, y, 16, theme->text);
  y += 25;
  DrawText("URL girin ve 'Git' butonuna basin veya", content.x, y, 12,
           theme->textDim);
  y += 18;
  DrawText("hizli erisim butonlarindan birini secin.", content.x, y, 12,
           theme->textDim);
  y += 25;
  DrawText("Harici tarayici (Firefox) acilacaktir.", content.x, y, 12,
           theme->textDim);

  // URL girişi
  if (win.id == activeWindowId) {
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 126) {
        win.browserUrl += (char)key;
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !win.browserUrl.empty()) {
      win.browserUrl.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER) ||
        (goHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
      if (!win.browserUrl.empty()) {
        std::string url = win.browserUrl;
        if (url.find("http") == std::string::npos) {
          url = "https://" + url;
        }
        std::string cmd = "firefox '" + url + "' &";
        system(cmd.c_str());
      }
    }
  }
}

void DrawWindowContent(Window &win, Vector2 mouse) {
  switch (win.appType) {
  case APP_FILES:
    DrawFileManager(win, mouse);
    break;
  case APP_SETTINGS:
    DrawSettings(win, mouse);
    break;
  case APP_TERMINAL:
    DrawTerminal(win, mouse);
    break;
  case APP_UPDATER:
    DrawUpdater(win, mouse);
    break;
  case APP_NOTEPAD:
    DrawNotepad(win, mouse);
    break;
  case APP_BROWSER:
    DrawBrowser(win, mouse);
    break;
  default:
    break;
  }
}

// ============================================================================
// PENCERE ETKİLEŞİMİ
// ============================================================================
void HandleWindowInteraction(Window &win, Vector2 mouse) {
  if (win.state == WIN_MINIMIZED)
    return;

  float headerH = 32;
  Rectangle header = {win.bounds.x, win.bounds.y, win.bounds.width, headerH};
  Rectangle resizeCorner = {win.bounds.x + win.bounds.width - 15,
                            win.bounds.y + win.bounds.height - 15, 15, 15};

  // Buton konumları
  float btnY = win.bounds.y + headerH / 2;
  float btnX = win.bounds.x + 16;
  float btnR = 8;
  float btnSpacing = 20;

  Rectangle closeBtn = {btnX - btnR, btnY - btnR, btnR * 2, btnR * 2};
  Rectangle minBtn = {btnX + btnSpacing - btnR, btnY - btnR, btnR * 2,
                      btnR * 2};
  Rectangle maxBtn = {btnX + btnSpacing * 2 - btnR, btnY - btnR, btnR * 2,
                      btnR * 2};

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (CheckCollisionPointRec(mouse, closeBtn)) {
      CloseWindow(win.id);
      return;
    }
    if (CheckCollisionPointRec(mouse, minBtn)) {
      win.state = WIN_MINIMIZED;
      return;
    }
    if (CheckCollisionPointRec(mouse, maxBtn)) {
      if (win.state == WIN_MAXIMIZED) {
        win.bounds = win.restoreBounds;
        win.state = WIN_NORMAL;
      } else {
        win.restoreBounds = win.bounds;
        win.bounds = {0, 32, (float)GetScreenWidth(),
                      (float)GetScreenHeight() - 32 - 75};
        win.state = WIN_MAXIMIZED;
      }
      return;
    }

    // Sürükleme başlat
    if (CheckCollisionPointRec(mouse, header) && win.state != WIN_MAXIMIZED) {
      win.isDragging = true;
      win.dragOffset = {mouse.x - win.bounds.x, mouse.y - win.bounds.y};
      BringToFront(win.id);
    }

    // Boyutlandırma başlat
    if (CheckCollisionPointRec(mouse, resizeCorner) &&
        win.state != WIN_MAXIMIZED) {
      win.isResizing = true;
      BringToFront(win.id);
    }

    // Pencereye tıklama
    if (CheckCollisionPointRec(mouse, win.bounds)) {
      BringToFront(win.id);
    }
  }

  // Sürükleme
  if (win.isDragging) {
    win.bounds.x = mouse.x - win.dragOffset.x;
    win.bounds.y = mouse.y - win.dragOffset.y;

    // Sınırlar
    win.bounds.y = Clamp(win.bounds.y, 32, GetScreenHeight() - 100);
  }

  // Boyutlandırma
  if (win.isResizing) {
    win.bounds.width =
        Clamp(mouse.x - win.bounds.x, 200, GetScreenWidth() - win.bounds.x);
    win.bounds.height = Clamp(mouse.y - win.bounds.y, 150,
                              GetScreenHeight() - win.bounds.y - 80);
  }

  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    win.isDragging = false;
    win.isResizing = false;
  }
}

// ============================================================================
// ÜST BAR
// ============================================================================
void DrawTopBar(Vector2 mouse) {
  int w = GetScreenWidth();

  // Bar arka planı
  DrawRectangle(0, 0, w, 32, theme->topBar);
  DrawRectangleGradientV(0, 32, w, 3, Fade(BLACK, 0.1f), Fade(BLACK, 0));

  // Logo
  DrawText("LumanovOS", 15, 8, 16, theme->text);

  // Saat
  time_t now = time(0);
  struct tm *ltm = localtime(&now);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M", ltm);
  DrawText(timeStr, w - 55, 8, 16, theme->text);

  // Sistem menü butonu
  Rectangle sysBtn = {(float)w - 90, 4, 25, 24};
  bool sysHover = CheckCollisionPointRec(mouse, sysBtn);
  DrawRectangleRounded(sysBtn, 0.3f, 8,
                       sysHover ? Fade(theme->accent, 0.3f) : Fade(WHITE, 0));
  DrawText("S", sysBtn.x + 8, sysBtn.y + 5, 14, theme->text);

  if (sysHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    showSystemMenu = !showSystemMenu;
  }

  // Sistem menüsü
  if (showSystemMenu) {
    Rectangle menu = {(float)w - 180, 36, 170, 130};
    DrawRectangleRounded(menu, 0.1f, 10, theme->windowBg);
    DrawRectangleRoundedLines(menu, 0.1f, 10, theme->border);

    const char *items[] = {"Guncelleme Kontrolu", "Yeniden Baslat",
                           "Sistemi Kapat"};
    Color colors[] = {theme->accent, theme->minimizeBtn, theme->closeBtn};

    for (int i = 0; i < 3; i++) {
      Rectangle item = {menu.x + 5, menu.y + 10 + i * 38, menu.width - 10, 32};
      bool hover = CheckCollisionPointRec(mouse, item);

      if (hover) {
        DrawRectangleRounded(item, 0.2f, 8, Fade(colors[i], 0.2f));
      }

      DrawCircle(item.x + 15, item.y + 16, 5, colors[i]);
      DrawText(items[i], item.x + 30, item.y + 9, 13, theme->text);

      if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showSystemMenu = false;
        if (i == 0) {
          CreateWindow("Guncelleme", APP_UPDATER, 300, 150, 350, 280);
        } else if (i == 1) {
          system("echo 120820 | sudo -S reboot");
        } else if (i == 2) {
          system("echo 120820 | sudo -S poweroff");
        }
      }
    }
  }
}

// ============================================================================
// DOCK
// ============================================================================
struct DockApp {
  const char *name;
  const char *icon;
  AppType appType;
  Color color;
};

DockApp dockApps[] = {
    {"Dosyalar", "D", APP_FILES, {0, 122, 255, 255}},
    {"Notepad", "N", APP_NOTEPAD, {255, 204, 0, 255}},
    {"Tarayici", "W", APP_BROWSER, {255, 87, 51, 255}},
    {"Terminal", ">", APP_TERMINAL, {40, 40, 45, 255}},
    {"Ayarlar", "A", APP_SETTINGS, {142, 142, 147, 255}},
    {"Guncelleme", "G", APP_UPDATER, {40, 200, 64, 255}},
};
int dockAppCount = 6;
float dockHoverAnim[6] = {0, 0, 0, 0, 0, 0};

void DrawDock(Vector2 mouse) {
  int w = GetScreenWidth();
  int h = GetScreenHeight();
  float dt = GetFrameTime();

  float dockW = dockAppCount * 65 + 30;
  float dockH = 60;
  float dockX = w / 2.0f - dockW / 2;
  float dockY = h - dockH - 10;

  // Dock arka planı
  DrawRectangleRounded({dockX, dockY, dockW, dockH}, 0.3f, 15, theme->dockBg);

  // Minimized pencereler için göstergeler (dock'un sağında)
  float minX = dockX + dockW + 15;
  for (auto &win : windows) {
    if (win.state == WIN_MINIMIZED) {
      Rectangle minRect = {minX, dockY + 10, 40, 40};
      bool hover = CheckCollisionPointRec(mouse, minRect);

      DrawRectangleRounded(minRect, 0.2f, 8,
                           hover ? Fade(theme->accent, 0.5f) : theme->border);
      DrawText(win.title.substr(0, 1).c_str(), minX + 14, dockY + 20, 16,
               theme->text);

      if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        win.state = WIN_NORMAL;
        BringToFront(win.id);
      }

      minX += 50;
    }
  }

  // Dock ikonları
  for (int i = 0; i < dockAppCount; i++) {
    float iconX = dockX + 20 + i * 65;
    float iconY = dockY + 8;
    Rectangle iconRect = {iconX, iconY, 45, 45};
    bool hover = CheckCollisionPointRec(mouse, iconRect);

    // Hover animasyonu
    dockHoverAnim[i] = Lerp(dockHoverAnim[i], hover ? 1.0f : 0.0f, dt * 10);
    float lift = dockHoverAnim[i] * 12;

    // İkon
    DrawRectangleRounded({iconX, iconY - lift, 45, 45}, 0.25f, 10,
                         dockApps[i].color);
    DrawText(dockApps[i].icon, iconX + 15, iconY + 12 - lift, 20, WHITE);

    // Tooltip
    if (dockHoverAnim[i] > 0.5f) {
      int tw = MeasureText(dockApps[i].name, 12);
      DrawRectangleRounded(
          {iconX + 22 - tw / 2.0f - 6, dockY - 28, (float)tw + 12, 20}, 0.4f, 8,
          Fade(BLACK, 0.8f));
      DrawText(dockApps[i].name, iconX + 22 - tw / 2.0f, dockY - 25, 12, WHITE);
    }

    // Tıklama
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      // Aynı tip pencere açık mı kontrol et
      bool exists = false;
      for (auto &win : windows) {
        if (win.appType == dockApps[i].appType) {
          win.state = WIN_NORMAL;
          BringToFront(win.id);
          exists = true;
          break;
        }
      }
      if (!exists) {
        // Uygulamaya göre pencere boyutu
        float winW = 450, winH = 350;
        if (dockApps[i].appType == APP_SETTINGS) {
          winW = 550;
          winH = 450;
        } else if (dockApps[i].appType == APP_NOTEPAD) {
          winW = 600;
          winH = 450;
        } else if (dockApps[i].appType == APP_BROWSER) {
          winW = 500;
          winH = 400;
        }
        CreateWindow(dockApps[i].name, dockApps[i].appType, 100 + i * 30,
                     80 + i * 30, winW, winH);
      }
    }

    // Açık pencere göstergesi
    for (auto &win : windows) {
      if (win.appType == dockApps[i].appType && win.state != WIN_MINIMIZED) {
        DrawCircle(iconX + 22, dockY + dockH - 3, 3, theme->accent);
        break;
      }
    }
  }
}

// ============================================================================
// ÖZEL MOUSE İMLECİ
// ============================================================================
void DrawCustomCursor(Vector2 pos) {
  Vector2 p1 = pos;
  Vector2 p2 = {pos.x, pos.y + 17};
  Vector2 p3 = {pos.x + 12, pos.y + 12};

  // Gölge
  DrawTriangle({p1.x + 2, p1.y + 2}, {p2.x + 2, p2.y + 2}, {p3.x + 2, p3.y + 2},
               Fade(BLACK, 0.3f));

  // İmleç
  DrawTriangle(p1, p2, p3, WHITE);
  DrawTriangleLines(p1, p2, p3, theme->accent);
}

// ============================================================================
// ANA DÖNGÜ
// ============================================================================
int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(GetScreenWidth(), GetScreenHeight(), "LumanovOS Desktop");
  SetTargetFPS(60);
  HideCursor();

  // Duvar kağıdı yükle
  DIR *dir = opendir("images");
  if (dir) {
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      std::string name = ent->d_name;
      if (name.find(".jpg") != std::string::npos ||
          name.find(".png") != std::string::npos) {
        wallpaper = LoadTexture(("images/" + name).c_str());
        break;
      }
    }
    closedir(dir);
  }

  // Hoşgeldin mesajı için zamanlayıcı
  float welcomeTimer = 3.0f;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    Vector2 mouse = GetMousePosition();

    // Hoşgeldin zamanlayıcısı
    if (welcomeTimer > 0)
      welcomeTimer -= dt;

    // Menü dışına tıklama
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (showSystemMenu) {
        Rectangle menuArea = {(float)GetScreenWidth() - 180, 0, 180, 170};
        if (!CheckCollisionPointRec(mouse, menuArea)) {
          showSystemMenu = false;
        }
      }
    }

    // Pencere etkileşimleri (ters sırada - üstteki önce)
    for (int i = windows.size() - 1; i >= 0; i--) {
      if (windows[i].state != WIN_MINIMIZED &&
          CheckCollisionPointRec(mouse, windows[i].bounds)) {
        HandleWindowInteraction(windows[i], mouse);
        break;
      }
    }

    // =============== ÇİZİM ===============
    BeginDrawing();

    // Arka plan
    if (wallpaper.id > 0) {
      DrawTexturePro(wallpaper,
                     {0, 0, (float)wallpaper.width, (float)wallpaper.height},
                     {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                     {0, 0}, 0, WHITE);
    } else {
      ClearBackground(theme->background);
    }

    // Üst bar
    DrawTopBar(mouse);

    // Pencereler
    for (auto &win : windows) {
      DrawWindowFrame(win, mouse);
      if (win.state != WIN_MINIMIZED) {
        DrawWindowContent(win, mouse);
      }
    }

    // Dock
    DrawDock(mouse);

    // Hoşgeldin mesajı
    if (welcomeTimer > 0 && windows.empty()) {
      float alpha = welcomeTimer < 1.0f ? welcomeTimer : 1.0f;
      float cx = GetScreenWidth() / 2.0f;
      float cy = GetScreenHeight() / 2.0f - 50;

      DrawRectangleRounded({cx - 180, cy - 40, 360, 80}, 0.1f, 15,
                           Fade(theme->windowBg, alpha));

      const char *msg = "LumanovOS'a Hosgeldiniz!";
      int msgW = MeasureText(msg, 22);
      DrawText(msg, cx - msgW / 2, cy - 10, 22, Fade(theme->text, alpha));
    }

    // Mouse imleci (en son)
    DrawCustomCursor(mouse);

    EndDrawing();
  }

  if (wallpaper.id > 0)
    UnloadTexture(wallpaper);
  CloseWindow();
  return 0;
}
