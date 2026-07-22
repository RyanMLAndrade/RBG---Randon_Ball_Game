#include "uiScreens.h"
#include "background.h"
#include "mapGenerator.h"
#include "playerCharacter.h"
#include "gameEngine.h"
#include "fontManager.h"
#include "readwrite.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

ScreenState currentScreen = Inicial;
static float uiFontScale = 0.3f;
static float uiFontReferenceDistance = 10.0f;
static int mainMenuIndex = 0;
static int configMenuIndex = 0;
static int levelSelectionIndex = 0;
static const int mainMenuCount = 3;
static const int configMenuCount = 3;
static const char* controlMethodNames[] = { "WASD", "Arrow", "Mouse" };
static const char* skinNames[] = { "Preta", "Laranja", "Rosa", "Fut", "Diamante" };
extern int skinPlayer;
static void drawCenteredTextAtDistance(const char* text, float yNorm, float distance);

static int getControlMethodIndex(void);
static const char* getControlMethodName(int index);
static void setControlMethodIndex(int index);

static void beginMenu2D(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void endMenu2D(void) {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static float getTextWidthNormalized(const char* text) {
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    if (windowWidth <= 0) return 0.0f;
    float textWidthPx = (float)getTextWidthPixels(text) * uiFontScale;
    return textWidthPx / (float)windowWidth * 2.0f;
}

static void drawCenteredText(const char* text, float yNorm) {
    int winW = glutGet(GLUT_WINDOW_WIDTH);
    int winH = glutGet(GLUT_WINDOW_HEIGHT);
    if (winW <= 0 || winH <= 0) return;

    int textW = getTextWidthPixels(text);
    float px = (winW - textW) * 0.5f;
    float py = (0.5f - yNorm * 0.5f) * winH;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
   
    glOrtho(0, winW, winH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawHDTextScaled(px, py, text, uiFontScale);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
static void drawStrokeTitle(const char* text, float yNorm) {
    int winW = glutGet(GLUT_WINDOW_WIDTH);
    int winH = glutGet(GLUT_WINDOW_HEIGHT);
    if (winW <= 0 || winH <= 0) return;

    int textW = getTextWidthPixels(text);
    float scale = 4.0f * uiFontScale; // tamanho da fonte
    float scaledW = textW * scale;
    float px = (winW - scaledW) * 0.5f;
    float py = (0.5f - yNorm * 0.5f) * winH;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, winW, winH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    


    glColor3f(0.0f, 0.0f, 0.0f);
    
    float ofs[] = { -5.0f,  5.0f,  0.0f,  0.0f };
    float ofy[] = {  0.0f,  0.0f, -5.0f,  5.0f };
    
    for (int i = 0; i < 4; i++) {
        //
        drawHDTextScaled(px + ofs[i], py + ofy[i], text, scale);
    }

    // Texto principal branco 
    glColor3f(1.0f, 0.8f, 0.3f);
    drawHDTextScaled(px, py, text, scale);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}
static void drawMenuButton(const char* text, float y, int selected) {
    float distance = 12.0f;
    int winW = glutGet(GLUT_WINDOW_WIDTH);
    if (winW <= 0) return;
    int textWpx = getTextWidthPixels(text);
    float simulatedScale = uiFontScale * (uiFontReferenceDistance / fmaxf(distance, 0.001f));
    float textWidthNorm = (textWpx * simulatedScale) / (float)winW * 2.0f;
    float paddingX = 0.06f;
    float paddingY = 0.08f;
    float left = -textWidthNorm * 0.5f - paddingX;
    float right = textWidthNorm * 0.5f + paddingX;
    float top = y + paddingY + 0.04f;
    float bottom = y - paddingY + 0.04f;

    glDisable(GL_TEXTURE_2D);
    if (selected) {
        glColor3f(0.90f, 0.70f, 0.30f);
    } else {
        glColor3f(0.72f, 0.55f, 0.40f);
    }
    glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();

    if (selected) {
        glColor3f(0.08f, 0.08f, 0.08f);
    } else {
        glColor3f(0.95f, 0.95f, 0.92f);
    }
    drawCenteredTextAtDistance(text, y - 0.015f, distance);

    // Restaura a cor padrão após desenhar o texto
    glColor3f(1.0f, 1.0f, 1.0f);
}

int getControlMethodIndex(void) {
    const char* value = getConfigValue("control");
    if (!value) {
        return 0; // Default para WASD
    }
    for (int i = 0; i < 3; i++) {
        if (strcmp(value, controlMethodNames[i]) == 0) {
            return i;
        }
    }
    return 0;
}

const char* getControlMethodName(int index) {
    if (index < 0 || index >= 3) {
        return "WASD";
    }
    return controlMethodNames[index];
}

void setControlMethodIndex(int index) {
    if (index < 0) index = 0;
    if (index >= 3) index = 2;
    setConfigValue("control", controlMethodNames[index]);
}

void menuNavigateUp(void) {
    if (currentScreen == Inicial) {
        mainMenuIndex = (mainMenuIndex + mainMenuCount - 1) % mainMenuCount;
    } else if (currentScreen == Configuracoes) {
        configMenuIndex = (configMenuIndex + configMenuCount - 1) % configMenuCount;
    }
}

void menuNavigateDown(void) {
    if (currentScreen == Inicial) {
        mainMenuIndex = (mainMenuIndex + 1) % mainMenuCount;
    } else if (currentScreen == Configuracoes) {
        configMenuIndex = (configMenuIndex + 1) % configMenuCount;
    }
}

static int getSelectableLevelCount(void) {
    int unlocked = getUnlockedLevel();
    int maxPreset = getMaxLevelPreset();
    return unlocked < maxPreset ? unlocked : maxPreset;
}

static void clampLevelSelectionIndex(void) {
    int count = getSelectableLevelCount();
    if (count <= 0) count = 1;
    if (levelSelectionIndex >= count) levelSelectionIndex = count - 1;
    if (levelSelectionIndex < 0) levelSelectionIndex = 0;
}

void menuChangeStateLeft(void) {
    if (currentScreen == Configuracoes) {
        if (configMenuIndex == 0) {
            int currentIndex = getControlMethodIndex();
            currentIndex = (currentIndex + 3 - 1) % 3;
            setControlMethodIndex(currentIndex);
        } else if (configMenuIndex == 1) {
            skinPlayer--;
            if (skinPlayer < 1) skinPlayer = 4;
            char valStr[16];
            snprintf(valStr, sizeof(valStr), "%d", skinPlayer);
            setConfigValue("p", valStr);
        }
    } else if (currentScreen == SelecaoLevel) {
        int count = getSelectableLevelCount();
        if (count > 0) {
            levelSelectionIndex = (levelSelectionIndex + count - 1) % count;
        }
    }
}

void menuChangeStateRight(void) {
    if (currentScreen == Configuracoes) {
        if (configMenuIndex == 0) {
            int currentIndex = getControlMethodIndex();
            currentIndex = (currentIndex + 1) % 3;
            setControlMethodIndex(currentIndex);
        } else if (configMenuIndex == 1) {
            skinPlayer++;
            if (skinPlayer > 4) skinPlayer = 1;
            char valStr[16];
            snprintf(valStr, sizeof(valStr), "%d", skinPlayer);
            setConfigValue("p", valStr);
        }
    } else if (currentScreen == SelecaoLevel) {
        int count = getSelectableLevelCount();
        if (count > 0) {
            levelSelectionIndex = (levelSelectionIndex + 1) % count;
        }
    }
}

void menuSelectOption(void) {
    if (currentScreen == Inicial) {
        if (mainMenuIndex == 0) {
            currentScreen = SelecaoLevel;
            levelSelectionIndex = getSelectedLevel() - 1;
            clampLevelSelectionIndex();
        } else if (mainMenuIndex == 1) {
            currentScreen = Configuracoes;
            configMenuIndex = 0;
            const char* valStr = getConfigValue("p");
            if (valStr) {
                int loadedSkin = atoi(valStr);
                if (loadedSkin >= 1 && loadedSkin <= 4) {
                    skinPlayer = loadedSkin;
                }
            }
        } else if (mainMenuIndex == 2) {
            resetPlayerPosition();
            currentScreen = Level;
        }
    } else if (currentScreen == Configuracoes) {
        if (configMenuIndex == 2) {
            currentScreen = Inicial;
        }
    } else if (currentScreen == SelecaoLevel) {
        clampLevelSelectionIndex();
        setSelectedLevel(levelSelectionIndex + 1);
        resetPlayerPosition();
        currentScreen = Level;
    }
}

static void drawInitialScreen() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    beginMenu2D();

    glColor3f(0.05f, 0.08f, 0.24f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();

    glColor3f(0.08f, 0.12f, 0.42f);
    glBegin(GL_QUADS);
        glVertex2f(-0.78f, -0.75f);
        glVertex2f(0.78f, -0.75f);
        glVertex2f(0.78f, 0.75f);
        glVertex2f(-0.78f, 0.75f);
    glEnd();

    // Título 
    drawStrokeTitle("RBG", 0.45f);

    // menu inicial
    drawMenuButton("Selecionar Level", 0.12f, mainMenuIndex == 0);
    drawMenuButton("Configuracoes", -0.18f, mainMenuIndex == 1);
    drawMenuButton("Jogar", -0.48f, mainMenuIndex == 2);
    drawCenteredTextAtDistance("Use as setas para navegar", -0.72f, 12.0f);

    endMenu2D();
    
}

static void drawConfigScreen() {
    const char* mouseValue = getConfigValue("mouse");
    if (!mouseValue) {
        mouseValue = "desconhecido";
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    beginMenu2D();

    glColor3f(0.08f, 0.10f, 0.16f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();

    glColor3f(0.15f, 0.18f, 0.28f);
    glBegin(GL_QUADS);
        glVertex2f(-0.82f, -0.68f);
        glVertex2f(0.82f, -0.68f);
        glVertex2f(0.82f, 0.68f);
        glVertex2f(-0.82f, 0.68f);
    glEnd();

    glColor3f(0.94f, 0.94f, 0.94f);
    drawCenteredTextAtDistance("Configuracoes", 0.45f, 12.0f);

    int controlIndex = getControlMethodIndex();
    char controlText[96];
    snprintf(controlText, sizeof(controlText), "Modo de controle  %s", getControlMethodName(controlIndex));
    drawMenuButton(controlText, 0.15f, configMenuIndex == 0);

    char skinText[96];
    int currentSkin = skinPlayer;
    if (currentSkin < 1 || currentSkin > 4) currentSkin = 1;
    snprintf(skinText, sizeof(skinText), "Personagem  %s", skinNames[currentSkin]);
    drawMenuButton(skinText, -0.10f, configMenuIndex == 1);

    drawCenteredTextAtDistance("Use as setas para navegar", -0.32f, 12.0f);
    drawMenuButton("Voltar", -0.50f, configMenuIndex == 2);

    drawCenteredTextAtDistance("Pressione Enter para confirmar selecion", -0.70f, 12.0f);

    endMenu2D();
}

static void drawSelectionScreen() {
    int unlockedLevel = getUnlockedLevel();
    int availableLevels = getSelectableLevelCount();
    clampLevelSelectionIndex();
    int selectedLevel = levelSelectionIndex + 1;
    if (selectedLevel > availableLevels) {
        selectedLevel = availableLevels;
        levelSelectionIndex = availableLevels - 1;
    }

    char levelText[64];
    char selectedText[64];
    snprintf(levelText, sizeof(levelText), "Level desbloqueado   %d", unlockedLevel);
    snprintf(selectedText, sizeof(selectedText), "Level selecionado   %d", selectedLevel);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    beginMenu2D();

    glColor3f(0.06f, 0.14f, 0.12f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();

    glColor3f(0.12f, 0.22f, 0.18f);
    glBegin(GL_QUADS);
        glVertex2f(-0.82f, -0.72f);
        glVertex2f(0.82f, -0.72f);
        glVertex2f(0.82f, 0.72f);
        glVertex2f(-0.82f, 0.72f);
    glEnd();

    glColor3f(0.94f, 0.94f, 0.94f);
    drawCenteredTextAtDistance("Selecao de Level", 0.32f, 12.0f);
    drawCenteredTextAtDistance(levelText, 0.12f, 12.0f);
    drawCenteredTextAtDistance(selectedText, -0.06f, 12.0f);
    drawCenteredTextAtDistance("Use as setas para escolher nivel", -0.18f, 12.0f);
    drawCenteredTextAtDistance("Pressione Enter para iniciar", -0.30f, 12.0f);
    drawCenteredTextAtDistance("Pressione b para voltar", -0.42f, 12.0f);

    endMenu2D();
}

static void drawGameScreen() {
    glDisable(GL_DEPTH_TEST);
        drawBackground();
        drawMountain();
        drawClouds();
    glEnable(GL_DEPTH_TEST);

    renderMap();
    drawPlayer();
    drawFog3D();
    drawHUD();
}

void drawTitleScreen() {
    drawInitialScreen();
}

void drawUIScreen() {
    if (currentScreen == Inicial) {
        drawInitialScreen();
    } else if (currentScreen == Configuracoes) {
        drawConfigScreen();
    } else if (currentScreen == SelecaoLevel) {
        drawSelectionScreen();
    } else {
        drawGameScreen();
    }
}

static void drawCenteredTextAtDistance(const char* text, float yNorm, float distance) {
    int winW = glutGet(GLUT_WINDOW_WIDTH);
    int winH = glutGet(GLUT_WINDOW_HEIGHT);
    if (winW <= 0 || winH <= 0) return;

    int textW = getTextWidthPixels(text);
    float simulatedScale = uiFontScale * (uiFontReferenceDistance / fmaxf(distance, 0.001f));
    float px = (winW - textW * simulatedScale) * 0.5f;
    float py = (0.5f - yNorm * 0.5f) * winH;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, winW, winH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawHDTextScaled(px, py, text, simulatedScale);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
