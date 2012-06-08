
#ifndef HUD_H
#define HUD_H

void hud_text_right(int x, int y);

void hud_text_left(float FPSB2, int it);

void draw_hud(int it);

void draw_info();

void time_string(int currtime, char *string, int length);

#endif
