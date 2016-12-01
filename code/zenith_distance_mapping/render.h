#ifndef RENDER_H
#define RENDER_H

void initialize(const unsigned int win_x, const unsigned int win_y, const char *const *const argv, const int argc);

int display(void);

void renderer_specific_commands(int key);

#endif//#ifndef RENDER_H
