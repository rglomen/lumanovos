import pygame
import os
import subprocess
import sys
import time
import math

# --- YARDIMCI FONKSİYONLAR ---
def get_sys_info():
    # Basit sistem bilgisi (CPU, RAM)
    try:
        cpu_usage = subprocess.check_output("top -bn1 | grep 'Cpu(s)' | awk '{print $2}'", shell=True).decode().strip()
        ram_usage = subprocess.check_output("free -m | awk 'NR==2{printf \"%.2f%%\", $3*100/$2 }'", shell=True).decode().strip()
        return f"CPU: {cpu_usage}% | RAM: {ram_usage}"
    except:
        return "Sistem bilgisi alınamadı."

# --- TEMEL KONFİGÜRASYON ---
BASE_PATH = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATH = os.path.join(BASE_PATH, "images")
FILE_MANAGER_PATH = os.path.join(BASE_PATH, "file_manager.py") # Dosya Yöneticisi kodu
FPS = 60

class LumanovosOS:
    def __init__(self):
        pygame.init()
        self.info = pygame.display.Info()
        self.screen = pygame.display.set_mode((self.info.current_w, self.info.current_h), pygame.FULLSCREEN)
        
        # Fareyi etkinleştir
        pygame.mouse.set_visible(True) 
        
        self.clock = pygame.time.Clock()

        # Fontlar
        self.font_title = pygame.font.SysFont("Montserrat", 70, bold=True)
        self.font_cat = pygame.font.SysFont("Montserrat", 45)
        self.font_item = pygame.font.SysFont("Montserrat", 35)
        self.font_small = pygame.font.SysFont("Monospace", 20)

        # Temalar (Dinamik renkler)
        self.themes = [
            {'name': 'Dark Nova', 'bg_primary': (10, 10, 20), 'bg_secondary': (30, 30, 50), 'text_primary': (0, 255, 200), 'text_secondary': (180, 180, 180), 'highlight': (0, 200, 150)},
            {'name': 'Light Sky', 'bg_primary': (220, 230, 240), 'bg_secondary': (200, 210, 220), 'text_primary': (0, 0, 80), 'text_secondary': (60, 60, 100), 'highlight': (0, 100, 200)},
            {'name': 'Blood Moon', 'bg_primary': (30, 0, 0), 'bg_secondary': (60, 0, 0), 'text_primary': (255, 100, 0), 'text_secondary': (200, 100, 0), 'highlight': (255, 50, 0)},
        ]
        self.current_theme_idx = 0
        self.theme = self.themes[self.current_theme_idx]

        self.categories = ["OYUNLAR", "MEDYA", "AYARLAR", "GÜÇ"]
        self.menu_items = {
            "OYUNLAR": ["Retro Arch", "Lumanovos Arcade", "Cloud Gaming"],
            "MEDYA":   ["Dosya Gezgini", "Resim Galerisi", "Müzik Çalar"],
            "AYARLAR": ["Tema Değiştir", "Ağ Durumu", "Sistem Bilgisi", "Sistemi Güncelle", "Çözünürlük Ayarı"],
            "GÜÇ":     ["Yeniden Başlat", "Sistemi Kapat", "Uyku Modu"]
        }
        
        self.cat_idx = 0
        self.item_idx = 0
        self.bg_images = self.load_backgrounds()
        self.current_bg_idx = 0
        self.bg_alpha = 0 # Arka plan geçiş animasyonu
        self.target_bg_alpha = 255
        self.bg_change_timer = 0 # Otomatik arka plan geçişi
        
        self.show_popup = False
        self.popup_msg = ""
        self.popup_time = 0

    def load_backgrounds(self):
        images = []
        if os.path.exists(IMAGE_PATH):
            for file in sorted(os.listdir(IMAGE_PATH)): # Resimleri alfabetik sirayla yukle
                if file.lower().endswith((".png", ".jpg", ".jpeg")):
                    try:
                        img = pygame.image.load(os.path.join(IMAGE_PATH, file)).convert_alpha()
                        img = pygame.transform.smoothscale(img, (self.info.current_w, self.info.current_h))
                        images.append(img)
                    except Exception as e:
                        print(f"Resim yukleme hatasi: {file}: {e}")
        return images

    def show_message(self, msg, duration=3):
        self.show_popup = True
        self.popup_msg = msg
        self.popup_time = time.time()
        
    def draw_background(self):
        if self.bg_images:
            current_image = self.bg_images[self.current_bg_idx]
            self.screen.blit(current_image, (0, 0))
            
            # Opaklık için ekstra yüzey
            overlay = pygame.Surface((self.info.current_w, self.info.current_h), pygame.SRCALPHA)
            overlay.fill((*self.theme['bg_primary'], 150)) # Hafif karartma
            self.screen.blit(overlay, (0,0))
            
        else:
            self.screen.fill(self.theme['bg_primary'])
            # Modern dalga efekti
            for i in range(0, self.info.current_w, 20):
                y = math.sin((i + self.bg_change_timer) * 0.005) * 50 + (self.info.current_h / 2)
                pygame.draw.circle(self.screen, self.theme['highlight'], (i, int(y)), 2)
            self.bg_change_timer += 0.5


    def handle_selection(self):
        sel = self.menu_items[self.categories[self.cat_idx]][self.item_idx]
        
        if sel == "Tema Değiştir":
            self.current_theme_idx = (self.current_theme_idx + 1) % len(self.themes)
            self.theme = self.themes[self.current_theme_idx]
            self.show_message(f"Tema: {self.theme['name']}")
            
        elif sel == "Ağ Durumu":
            try:
                ip = subprocess.check_output(["hostname", "-I"]).decode().split()[0]
                self.show_message(f"Yerel IP: {ip}")
            except:
                self.show_message("Ağ Bağlantısı Yok veya IP Alınamadı!")
                
        elif sel == "Sistem Bilgisi":
            self.show_message(get_sys_info())

        elif sel == "Dosya Gezgini":
            if os.path.exists(FILE_MANAGER_PATH):
                pygame.quit() # Ana uygulamayı kapat
                subprocess.run(['python3', FILE_MANAGER_PATH]) # Dosya yöneticisini başlat
                # Dosya yöneticisinden dönünce tekrar başla
                pygame.init()
                self.screen = pygame.display.set_mode((self.info.current_w, self.info.current_h), pygame.FULLSCREEN)
                pygame.mouse.set_visible(True)
            else:
                self.show_message("Dosya Yöneticisi Bulunamadı!")

        elif sel == "Sistemi Güncelle":
            self.show_message("Güncelleme Denetleniyor... (Updater yeniden başlatılır)")
            subprocess.run(["sudo", "reboot"]) # Updater'ın tekrar çalışması için yeniden başlat

        elif sel == "Sistemi Kapat":
            subprocess.run(["sudo", "poweroff"])
        elif sel == "Yeniden Başlat":
            subprocess.run(["sudo", "reboot"])
        
    def render(self):
        self.draw_background()
        
        # Sol Panel (Sabit başlık)
        pygame.draw.rect(self.screen, self.theme['bg_secondary'], (0, 0, 300, self.info.current_h))
        title_surf = self.font_title.render("Lumanovos", True, self.theme['text_primary'])
        self.screen.blit(title_surf, (30, 50))
        
        # KATEGORİLER (Sol Panelde)
        for i, cat in enumerate(self.categories):
            is_selected = (i == self.cat_idx)
            color = self.theme['highlight'] if is_selected else self.theme['text_secondary']
            
            txt_surf = self.font_cat.render(cat, True, color)
            self.screen.blit(txt_surf, (50, 200 + (i*80)))

        # ÖĞELER (Sağdaki büyük liste)
        curr_cat_name = self.categories[self.cat_idx]
        items = self.menu_items[curr_cat_name]
        
        for j, item in enumerate(items):
            is_selected = (j == self.item_idx)
            color = self.theme['text_primary'] if is_selected else self.theme['text_secondary']
            
            txt_surf = self.font_item.render(item, True, color)
            self.screen.blit(txt_surf, (350, 150 + (j*70)))
            
            # Seçim oku animasyonu
            if is_selected:
                arrow = self.font_item.render(">", True, self.theme['highlight'])
                self.screen.blit(arrow, (310, 150 + (j*70)))

        # ALT BİLGİ
        status_text = self.font_small.render(f"v2.0 | {time.strftime('%H:%M')} | {get_sys_info()}", True, self.theme['text_secondary'])
        self.screen.blit(status_text, (30, self.info.current_h - 50))

        # Popup Mesajı
        if self.show_popup:
            if time.time() - self.popup_time < 3:
                popup_w, popup_h = 600, 100
                popup_x = (self.info.current_w - popup_w) // 2
                popup_y = self.info.current_h - popup_h - 50
                pygame.draw.rect(self.screen, self.theme['bg_primary'], (popup_x, popup_y, popup_w, popup_h), 0, 10)
                pygame.draw.rect(self.screen, self.theme['highlight'], (popup_x, popup_y, popup_w, popup_h), 3, 10)
                msg_surf = self.font_small.render(self.popup_msg, True, self.theme['text_primary'])
                self.screen.blit(msg_surf, msg_surf.get_rect(center=(popup_x + popup_w/2, popup_y + popup_h/2)))
            else:
                self.show_popup = False


        pygame.display.flip()

    def run(self):
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_RIGHT:
                        self.cat_idx = (self.cat_idx + 1) % len(self.categories)
                        self.item_idx = 0
                    elif event.key == pygame.K_LEFT:
                        self.cat_idx = (self.cat_idx - 1) % len(self.categories)
                        self.item_idx = 0
                    elif event.key == pygame.K_DOWN:
                        self.item_idx = (self.item_idx + 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    elif event.key == pygame.K_UP:
                        self.item_idx = (self.item_idx - 1) % len(self.menu_items[self.categories[self.cat_idx]])
                    elif event.key == pygame.K_RETURN:
                        self.handle_selection()
                    elif event.key == pygame.K_ESCAPE: # Kapatma icin ESC
                        self.show_message("Kapatmak icin GUC -> Sistemi Kapat'i secin.", 5)
                        
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    # Fare ile tıklama algılama (basit)
                    x, y = event.pos
                    # Kategori tıklama
                    for i, cat in enumerate(self.categories):
                        if pygame.Rect(50, 200 + (i*80), 200, 60).collidepoint(x, y):
                            self.cat_idx = i
                            self.item_idx = 0
                            break
                    # Öğelere tıklama
                    curr_cat_name = self.categories[self.cat_idx]
                    items = self.menu_items[curr_cat_name]
                    for j, item in enumerate(items):
                        if pygame.Rect(350, 150 + (j*70), 400, 50).collidepoint(x, y):
                            self.item_idx = j
                            self.handle_selection()
                            break

            self.render()
            self.clock.tick(FPS)

if __name__ == "__main__":
    app = LumanovosOS()
    app.run()
