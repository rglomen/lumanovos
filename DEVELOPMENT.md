# LumanovOS Geliştirme Kılavuzu

Bu döküman, LumanovOS masaüstü ortamının nasıl geliştirildiğini ve nasıl genişletilebileceğini açıklar.

## 📋 İçindekiler

1. [Mimari Genel Bakış](#mimari-genel-bakış)
2. [UI Kütüphanesi](#ui-kütüphanesi)
3. [Yeni Uygulama Ekleme](#yeni-uygulama-ekleme)
4. [Tema Sistemi](#tema-sistemi)
5. [Derleme Süreci](#derleme-süreci)
6. [Hata Ayıklama](#hata-ayıklama)

---

## 🏗️ Mimari Genel Bakış

LumanovOS modüler bir mimariye sahiptir:

```
┌─────────────────────────────────────────┐
│           Masaüstü Shell                │
│  (core/shell.cpp - lumanovos-shell)     │
├─────────────────────────────────────────┤
│                                         │
│   ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐      │
│   │Files│ │Sett.│ │Term.│ │Note.│ ...  │
│   └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘      │
│      │       │       │       │          │
├──────┴───────┴───────┴───────┴──────────┤
│           Ortak UI Kütüphanesi          │
│           (libs/ui/ui.cpp)              │
├─────────────────────────────────────────┤
│              Raylib                      │
└─────────────────────────────────────────┘
```

### Bileşenler

| Bileşen | Dosya | Açıklama |
|---------|-------|----------|
| Shell | `core/shell.cpp` | Ana masaüstü, dock, panel yönetimi |
| UI Lib | `libs/ui/ui.cpp` | Ortak UI bileşenleri |
| Apps | `apps/*/main.cpp` | Her uygulama kendi binary'si |

---

## 🎨 UI Kütüphanesi

### Kullanım

```cpp
#include "libs/ui/ui.h"

int main() {
    InitWindow(800, 600, "Uygulamam");
    HideCursor();  // Özel imleç için
    LUI::SetTheme(true);  // Koyu tema
    
    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        BeginDrawing();
        ClearBackground(LUI::currentTheme->windowBg);
        
        // UI bileşenlerini kullan
        if (LUI::Button({10, 10, 100, 30}, "Tikla")) {
            // Buton tıklandı
        }
        
        DrawCustomCursor(mouse);  // İmleci çiz
        EndDrawing();
    }
    
    ShowCursor();
    CloseWindow();
    return 0;
}
```

### Mevcut Bileşenler

| Fonksiyon | Açıklama |
|-----------|----------|
| `LUI::Button(rect, text, disabled)` | Tıklanabilir buton |
| `LUI::TextBox(rect, text, maxLen, focused)` | Metin girişi |
| `LUI::ListItem(rect, text, selected, icon)` | Liste öğesi |
| `LUI::Slider(rect, value, min, max)` | Kaydırıcı |
| `LUI::Checkbox(rect, label, checked)` | Onay kutusu |
| `LUI::Toggle(rect, value)` | Açma/kapama düğmesi |
| `LUI::ProgressBar(rect, progress, color)` | İlerleme çubuğu |
| `LUI::Separator(x, y, width)` | Ayırıcı çizgi |
| `LUI::DrawWindowFrame(bounds, title, active)` | Pencere çerçevesi |
| `LUI::DrawDock(items, count, clicked)` | Alt dock |
| `LUI::DrawTopPanel(name, menuClicked)` | Üst panel |

### Mouse İmleci

Her uygulamaya özel imleç eklemek için:

```cpp
void DrawCustomCursor(Vector2 pos) {
  DrawTriangle(pos, {pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, WHITE);
  DrawLineEx(pos, {pos.x, pos.y + 18}, 1.5f, BLACK);
  DrawLineEx(pos, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
  DrawLineEx({pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
}
```

---

## ➕ Yeni Uygulama Ekleme

### 1. Klasör Oluştur

```bash
mkdir -p apps/myapp
```

### 2. main.cpp Yaz

```cpp
#include "libs/ui/ui.h"

void DrawCustomCursor(Vector2 pos) {
  DrawTriangle(pos, {pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, WHITE);
  DrawLineEx(pos, {pos.x, pos.y + 18}, 1.5f, BLACK);
  DrawLineEx(pos, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
  DrawLineEx({pos.x, pos.y + 18}, {pos.x + 12, pos.y + 12}, 1.5f, BLACK);
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(600, 400, "Uygulamam");
  SetTargetFPS(60);
  HideCursor();
  LUI::SetTheme(true);
  
  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    
    BeginDrawing();
    ClearBackground(LUI::currentTheme->windowBg);
    
    LUI::WindowResult res = LUI::DrawWindowFrame(
        {0, 0, (float)w, (float)h}, "Uygulamam", true);
    if (res.closeClicked) break;
    
    // Uygulama içeriği buraya
    DrawText("Merhaba!", 50, 60, 20, LUI::currentTheme->text);
    
    DrawCustomCursor(mouse);
    EndDrawing();
  }
  
  ShowCursor();
  CloseWindow();
  return 0;
}
```

### 3. build.sh Güncelle

`build.sh` dosyasındaki döngüye uygulamanı ekle:

```bash
for app in files settings terminal notepad browser monitor myapp; do
```

### 4. Shell'e Ekle (Opsiyonel)

Dock'a eklemek için `core/shell.cpp`:

```cpp
// Enum'a ekle
enum AppType { ..., APP_MYAPP };

// Path'e ekle
const char *appPaths[] = { ..., "./apps/myapp/myapp" };

// DockItem'a ekle
LUI::DockItem dockItems[] = {
    ...,
    {"Uygulamam", "A", {100, 150, 200, 255}, false, 0},
};
const int dockItemCount = 7;  // Sayıyı artır
```

---

## 🎨 Tema Sistemi

### Tema Yapısı

```cpp
struct Theme {
  Color background;       // Ana arkaplan
  Color windowBg;         // Pencere arkaplanı
  Color windowHeader;     // Pencere başlığı
  Color windowHeaderActive;
  Color panelBg;          // Panel arkaplanı
  Color dockBg;           // Dock arkaplanı
  Color text;             // Ana metin
  Color textDim;          // Soluk metin
  Color accent;           // Vurgu rengi
  Color closeBtn;         // Kapat butonu
  Color minimizeBtn;      // Simge durumuna
  Color maximizeBtn;      // Tam ekran
  Color border;           // Kenar
  Color inputBg;          // Giriş alanı
  Color buttonBg;         // Buton
  Color buttonHover;      // Buton hover
  Color success;          // Başarı
  Color warning;          // Uyarı
  Color error;            // Hata
};
```

### Tema Değiştirme

```cpp
LUI::SetTheme(true);   // Koyu tema
LUI::SetTheme(false);  // Açık tema

// Accent rengi değiştirme
LUI::currentTheme->accent = {255, 0, 128, 255};
```

---

## 🔧 Derleme Süreci

### Manuel Derleme

```bash
# UI kütüphanesi
g++ -O2 -Wall -c libs/ui/ui.cpp -o libs/ui/ui.o

# Shell
g++ -O2 -Wall core/shell.cpp libs/ui/ui.o -o bin/lumanovos-shell \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Uygulama
g++ -O2 -Wall -I. apps/files/main.cpp libs/ui/ui.o -o apps/files/files \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### Otomatik Derleme

```bash
./build.sh
```

---

## 🐛 Hata Ayıklama

### Yaygın Hatalar

| Hata | Çözüm |
|------|-------|
| `raylib.h not found` | `sudo apt install libraylib-dev` |
| `undefined reference to LUI::` | `-I.` flag ekle veya include path düzelt |
| `No such file or directory` | `libs/ui/ui.h` include path kontrol et |
| Mouse görünmüyor | `HideCursor()` ve `DrawCustomCursor()` ekle |

### Log Dosyaları

```bash
cat ~/build_err.txt       # Derleme hataları
cat ~/desktop_error.log   # Çalışma zamanı hataları
```

---

## 📚 Geliştirme Geçmişi

### v1.0.0 (2026-01-10)

- Modüler mimari: monolitik yapıdan ayrı binary'lere geçiş
- Ortak UI kütüphanesi (`libs/ui/`)
- 6 dahili uygulama: Files, Settings, Terminal, Notepad, Browser, Monitor
- Özel mouse imleci
- 5 tab'lı ayarlar menüsü (Görünüm, Arkaplan, Ses, Sistem, Hakkında)
- Koyu/Açık tema desteği
- Accent renk seçimi
- Wallpaper değiştirme

---

**Sorular için**: GitHub Issues kullanabilirsiniz.
