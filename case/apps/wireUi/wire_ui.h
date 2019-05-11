#ifndef __WIRE_UI_H
#define __WIRE_UI_H

int wireUI_DeInit(int *width, int *height);

void *getDeHandle();

void *getUIpHeap();

int wireUI_Init();

int wireUI_LoadPic(char *jpgPath, int x, int y, int width, int height);

int wireUI_Flip();

int wireUI_Hide();

void wireUI_DeRelease();

void wireUI_Release();

#endif
