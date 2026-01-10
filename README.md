# LumanovOS Desktop Environment

Modern, modular desktop environment built with Raylib.

## 🚀 Features

- **Modular Design**: Each application runs as a separate binary
- **Shared UI Library**: Consistent look and feel across all apps
- **Theme Support**: Dark and Light themes
- **6 Built-in Applications**: Files, Settings, Terminal, Notepad, Browser, Monitor

## 📦 Structure

```
lumanovos/
├── libs/ui/          # Shared UI library
├── core/             # Main shell
├── apps/             # Applications
│   ├── files/        # File manager
│   ├── settings/     # System settings
│   ├── terminal/     # Terminal emulator
│   ├── notepad/      # Text editor
│   ├── browser/      # Web browser launcher
│   └── monitor/      # System monitor
├── assets/           # Icons, wallpapers, fonts
└── config/           # Configuration files
```

## 🔨 Build

```bash
chmod +x build.sh
./build.sh
```

## 🎯 Run

```bash
./bin/lumanovos-shell
```

## 📋 Requirements

- Linux (Debian/Ubuntu based)
- Raylib 5.x
- GCC/G++
- X11

## 📜 License

LumanovOS Desktop - A lightweight desktop environment