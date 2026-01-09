import pygame
import os
import sys

class FileManager:
    def __init__(self):
        pygame.init()
        self.info = pygame.display.Info()
        self.screen = pygame.display.set_mode((self.info.current_w, self.info.current_h), pygame.FULLSCREEN)
        pygame.mouse.set_visible(True)
        self.clock = pygame.time.Clock()
        
        self.font_title = pygame.font.SysFont("Montserrat", 40, bold=True)
        self.font_item = pygame.font.SysFont("Montserrat", 25)
        
        self.current_dir = os.path.expanduser('~') # Başlangıç: Kullanıcının ana dizini
        self.files = self.get_files_in_dir(self.current_dir)
        self.selected_idx = 0
        
        self.bg_color = (30, 30, 40)
        self.panel_color = (50, 50, 60)
        self.text_color = (200, 200, 200)
        self.highlight_color = (0, 180, 255)

    def get_files_in_dir(self, path):
        try:
            items = sorted(os.listdir(path), key=lambda s: s.lower())
            dirs = [d for d in items if os.path.isdir(os.path.join(path, d))]
            files = [f for f in items if os.path.isfile(os.path.join(path, f))]
            return [".. (Üst Dizin)"] + dirs + files
        except Exception as e:
            print(f"Dizin okuma hatasi: {e}")
            return [".. (Üst Dizin)", "Hata: Dizin okunamadı."]

    def render(self):
        self.screen.fill(self.bg_color)
        
        # Üst Panel (Dizin yolu)
        pygame.draw.rect(self.screen, self.panel_color, (0, 0, self.info.current_w, 80))
        path_text = self.font_title.render(self.current_dir, True, self.text_color)
        self.screen.blit(path_text, (20, 20))
        
        # Dosya/Dizin Listesi
        for i, item in enumerate(self.files):
            y_pos = 100 + (i * 40)
            
            if i == self.selected_idx:
                pygame.draw.rect(self.screen, self.highlight_color, (0, y_pos - 5, self.info.current_w, 40), 0, 5)
                color = (255, 255, 255)
            else:
                color = self.text_color
            
            txt_surf = self.font_item.render(item, True, color)
            self.screen.blit(txt_surf, (30, y_pos))

        # Alt Panel (Tuş bilgisi)
        pygame.draw.rect(self.screen, self.panel_color, (0, self.info.current_h - 50, self.info.current_w, 50))
        help_text = self.font_item.render("[ESC] Geri | [ENTER] Aç", True, self.text_color)
        self.screen.blit(help_text, (self.info.current_w - help_text.get_width() - 20, self.info.current_h - 40))

        pygame.display.flip()

    def handle_selection(self):
        if not self.files: return

        selected_item = self.files[self.selected_idx]
        
        if selected_item == ".. (Üst Dizin)":
            self.current_dir = os.path.abspath(os.path.join(self.current_dir, os.pardir))
        else:
            item_path = os.path.join(self.current_dir, selected_item)
            if os.path.isdir(item_path):
                self.current_dir = item_path
            elif os.path.isfile(item_path):
                # Dosya açma (örneğin resimleri veya metinleri)
                self.show_message(f"Dosya Açıldı: {selected_item}") # Şu an sadece mesaj veriyor
                return
        
        self.files = self.get_files_in_dir(self.current_dir)
        self.selected_idx = 0

    def show_message(self, msg):
        # Basit mesaj kutusu
        print(msg) # Terminale de bas

    def run(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    elif event.key == pygame.K_DOWN:
                        self.selected_idx = (self.selected_idx + 1) % len(self.files) if self.files else 0
                    elif event.key == pygame.K_UP:
                        self.selected_idx = (self.selected_idx - 1) % len(self.files) if self.files else 0
                    elif event.key == pygame.K_RETURN:
                        self.handle_selection()
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    # Fare ile tıklama
                    x, y = event.pos
                    # Öğe seçimi
                    for i, item in enumerate(self.files):
                        rect = pygame.Rect(0, 100 + (i * 40) - 5, self.info.current_w, 40)
                        if rect.collidepoint(x, y):
                            self.selected_idx = i
                            if event.button == 1: # Sol tık
                                self.handle_selection()
                                break
            
            self.render()
            self.clock.tick(60)

if __name__ == "__main__":
    fm = FileManager()
    fm.run()
    pygame.quit()
    sys.exit()
