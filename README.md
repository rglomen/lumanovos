# LumanovOS Desktop Environment

Modern, modüler masaüstü ortamı. Raylib ile geliştirildi.

![LumanovOS Logo](assets/logo.png)

## 🌟 Özellikler

- **Modüler Mimari**: Her uygulama ayrı binary olarak çalışır
- **Ortak UI Kütüphanesi**: Tutarlı tasarım ve tema desteği
- **Koyu/Açık Tema**: Kullanıcı tercihine göre değiştirilebilir
- **Özel Mouse İmleci**: Tüm uygulamalarda tutarlı görünüm
- **6 Dahili Uygulama**: Files, Settings, Terminal, Notepad, Browser, Monitor

## 📁 Proje Yapısı

```
lumanovos/
├── libs/ui/              # Ortak UI kütüphanesi
│   ├── ui.h              # Header dosyası
│   └── ui.cpp            # Implementasyon
├── core/
│   └── shell.cpp         # Ana masaüstü (dock, panel, menü)
├── apps/                 # Uygulamalar
│   ├── files/            # Dosya Yöneticisi
│   ├── settings/         # Ayarlar (5 tab)
│   ├── terminal/         # Terminal Emülatörü
│   ├── notepad/          # Metin Editörü
│   ├── browser/          # Web Tarayıcı Başlatıcı
│   └── monitor/          # Sistem Monitörü
├── assets/               # Görseller ve fontlar
│   ├── icons/            # Uygulama ikonları
│   └── wallpapers/       # Arkaplan resimleri
├── images/               # Ek resimler
├── config/               # Yapılandırma dosyaları
├── build.sh              # Derleme scripti
├── launcher.cpp          # Boot sonrası launcher
└── version.txt           # Versiyon numarası
```

## 🔨 Derleme

### Gereksinimler

- Linux (Debian/Ubuntu tabanlı)
- GCC/G++ derleyici
- Raylib 5.x kütüphanesi
- X11 geliştirme paketleri

### Kurulum

```bash
# Raylib kurulumu (Ubuntu/Debian)
sudo apt update
sudo apt install libraylib-dev

# Projeyi klonla
git clone https://github.com/rglomen/lumanovos.git
cd lumanovos

# Derle
chmod +x build.sh
./build.sh
```

### Çalıştırma

```bash
./bin/lumanovos-shell
```

## 🎯 Uygulamalar

| Uygulama | Açıklama | Kısayol |
|----------|----------|---------|
| **Files** | Dosya gezgini, klasör navigasyonu | F |
| **Settings** | Tema, arkaplan, ses, sistem ayarları | S |
| **Terminal** | Komut satırı, shell erişimi | > |
| **Notepad** | Metin düzenleme, dosya kaydetme | N |
| **Browser** | Firefox ile web sayfası açma | W |
| **Monitor** | CPU, RAM, Disk kullanımı | M |

## ⚙️ Ayarlar Menüsü

1. **Görünüm**: Koyu/Açık tema, accent renk, parlaklık
2. **Arkaplan**: Wallpaper seçimi
3. **Ses**: Ana ses, müzik, efekt seviyeleri
4. **Sistem**: Otomatik başlatma, saat formatı, yeniden başlat/kapat
5. **Hakkında**: Versiyon, sistem bilgileri (CPU, RAM, Disk, Kernel)

## 🖱️ Kullanım

- **Dock**: Ekranın altında, uygulamaları başlatmak için tıkla
- **Üst Panel**: Sol üstte sistem adı, ortada saat, sağda sistem menüsü
- **Sistem Menüsü**: Tema değiştirme, ayarlar, yeniden başlat, kapat

## 📝 Geliştirme

Detaylı geliştirme bilgisi için: [DEVELOPMENT.md](DEVELOPMENT.md)

## 📜 Lisans

MIT License - Serbestçe kullanın ve değiştirin.

---

**LumanovOS** - Hafif, hızlı, modern masaüstü deneyimi.