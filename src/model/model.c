#include "model.h"
#include <lcom/lcf.h>

// Variáveis externas importantes à construção e manipulação do modelo
extern uint8_t scancode;
extern uint8_t packet_counter;
SystemState systemState = RUNNING;
MenuState menuState = START;
extern vbe_mode_info_t mode_info;

Sprite* mouse;

void setup_sprites() {
  mouse = create_sprite_xpm((xpm_map_t) mouse_xpm);
}

void destroy_sprites() {
  destroy_sprite(mouse);
}

void update_keyboard_state() {
    (kbc_ih)();
    switch (scancode) {
        case ESC_BK_CODE:
            systemState = EXIT;
            break;
            }
}

void update_mouse_state() {
    mouse_ih();
    mouse_sync();
    if (packet_counter == 3) {
        sync_mouse_info();
    }

}

void update_timer_state() {
    vg_set_start();
    clear_vram();
    draw_new_frame();
}
