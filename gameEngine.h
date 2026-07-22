#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

// Inicialização e Loop Principal
void init();
void timer(int v);

//Teclado Especial/ Setas
void specialDown(int key, int x, int y);
void specialUp(int key, int x, int y);
void resetPlayerPosition();

//Teclado Normal
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);

// Mouse
void mouseMotion(int x, int y);
void mouseClick(int button, int state, int x, int y);
void drawHUD();
int isEndLevelScreenActive(void);
void gerarTexturaSombra();

#endif
