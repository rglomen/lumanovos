import pygame
import os
import subprocess
import sys
import time

# --- DONANIM KONTROL FONKSİYONLARI ---
def set_volume(change):
    # Ses seviyesini %5 artır veya azalt
    sign = "+" if change > 0 else "-"
    subprocess.run(["amixer", "-q", "set", "Master", f"5%{sign}"], check=False)

def toggle_mic():
    # Mikrofonu sustur/aç
    subprocess.run(["amixer", "set", "Capture", "toggle"], check=False)

def get_current_volume():
    try:
        output = subprocess.check_output("amixer get Master | grep -o '[0-9]*%' | head -1", shell=True).decode().strip()
        return output
    except: return "??%"

def change_resolution(res):
    # Örnek: "1280x720"
    subprocess.run(["xrandr", "-s", res], check=False)

# --- ANA DASHBOARD GÜNCELLEMESİ ---
# (Önceki kodun üzerine bu yeni handle_selection mantığını ekliyoruz)

    def handle_selection(self):
        category = self.categories[self.cat_idx]
        sel = self.menu_items[category][self.item_idx]
        
        # --- GELİŞMİŞ AYARLAR MANTIĞI ---
        if category == "AYARLAR":
            if sel == "Ses +":
                set_volume(5)
                self.show_message(f"Ses Seviyesi: {get_current_volume()}")
            elif sel == "Ses -":
                set_volume(-5)
                self.show_message(f"Ses Seviyesi: {get_current_volume()}")
            elif sel == "Mikrofonu Kapat/Aç":
                toggle_mic()
                self.show_message("Mikrofon Durumu Değişti")
            elif sel == "720p Modu":
                change_resolution("1280x720")
                self.show_message("Çözünürlük: 720p")
            elif sel == "1080p Modu":
                change_resolution("1920x1080")
                self.show_message("Çözünürlük: 1080p")
            elif sel == "Tema Değiştir":
                self.current_theme_idx = (self.current_theme_idx + 1) % len(self.themes)
                self.theme = self.themes[self.current_theme_idx]
        
        # --- DİĞER KONTROLLER ---
        elif category == "GÜÇ":
            if sel == "Sistemi Kapat": subprocess.run(["sudo", "poweroff"])
            elif sel == "Yeniden Başlat": subprocess.run(["sudo", "reboot"])
