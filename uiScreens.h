#ifndef UI_SCREENS_H
#define UI_SCREENS_H

typedef enum {
    Inicial,
    Configuracoes,
    SelecaoLevel,
    Level
} ScreenState;

extern ScreenState currentScreen;

void drawTitleScreen();
void drawUIScreen();

void menuNavigateUp(void);
void menuNavigateDown(void);
void menuChangeStateLeft(void);
void menuChangeStateRight(void);
void menuSelectOption(void);

#endif
