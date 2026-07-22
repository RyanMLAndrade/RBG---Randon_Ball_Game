#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "gameEngine.h"
#include "mapGenerator.h"
#include "constants.h"
#include "uiScreens.h"
#include "readwrite.h"
#include "fontManager.h"
#include "drawObject.h"

float cameraX = 1.5f, cameraZ = 1.0f, cameraY = 1.2f;
float speed = 0.15f, lateralSpeed = 0.15f;
float maxSpeed = 0.30f, minSpeed = 0.15f;
float accelVertical = 0.002f, dragVertical = 0.0005f;
float accelLateral = 0.003f, maxLateralSpeed = 0.055f;
float blockWidth = 0.6f;
static bool deadControlLocked = false;

float jumpSpeed = 0.0f;
float gravity = 0.003f;
float groundY = 1.2f;
float jumpAscendForce = 0.007f; 
float maxJumpSpeed = 0.06f;
bool isJumping = false;
bool canAscend = false; 

int jumpBufferTimer = 0;
const int JUMP_BUFFER_MAX = 10; 

int jumpTimer = 0;
int minJumpFrames = 8;  
int maxJumpFrames = 25; 

bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
bool mouseSpecialDown = false;
bool mouseControlEnabled = true;
float mouseXPercent = 0.0f;
int mouseSteerFrame = 0;

bool isDead = false;
int deathTimer = 0;
int skinPlayer = 1;
static float deathFallSpeed = 0.0f;
float deathBallYOffset = 0.0f;
static float platformCoverage = 0.0f;
static float platformCoverageLeft = 0.0f;
static float platformCoverageRight = 0.0f;

static bool endLevelScreen = false;
static bool isVictory = false;
static int endLevelMenuIndex = 0;
static const int endLevelMenuCount = 3;

GLuint texturaSombraID;
void gerarTexturaSombra() {
    int tamanho = 64; 
    GLubyte pixels[64][64][4]; 

    float centro = (tamanho - 1) / 2.0f;

    for (int y = 0; y < tamanho; y++) {
        for (int x = 0; x < tamanho; x++) {
            float dx = x - centro;
            float dy = y - centro;
            float distancia = sqrtf(dx*dx + dy*dy) / centro;

            float alpha = 1.0f - distancia;
            if (alpha < 0.0f) alpha = 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;

            alpha = alpha * alpha * 0.8f; 

            pixels[y][x][0] = 0;   
            pixels[y][x][1] = 0;   
            pixels[y][x][2] = 0;   
            pixels[y][x][3] = (GLubyte)(alpha * 255); 
        }
    }

    glGenTextures(1, &texturaSombraID);
    glBindTexture(GL_TEXTURE_2D, texturaSombraID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tamanho, tamanho, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

static void updatePlatformCoverage() {
    float height = cameraY - groundY;
    float shadowZPos = cameraZ + 3.65f;
    float radius = 0.21f + (height * 0.05f);
    if (radius < 0.01f) radius = 0.01f;

    float shadowMinX = cameraX - radius;
    float shadowMaxX = cameraX + radius;
    float shadowMinZ = shadowZPos - radius;
    float shadowMaxZ = shadowZPos + radius;
    float totalArea = (shadowMaxX - shadowMinX) * (shadowMaxZ - shadowMinZ);
    if (totalArea <= 0.0f) {
        platformCoverage = 0.0f;
        platformCoverageLeft = 0.0f;
        platformCoverageRight = 0.0f;
        return;
    }

    float insideArea = 0.0f;
    float insideLeftArea = 0.0f;
    float insideRightArea = 0.0f;

    int rowCentro = (int)floorf(-shadowZPos);
    int colCentro = (int)floorf(cameraX / blockWidth);

    for (int r = rowCentro; r <= rowCentro + 1; r++) {
        float rowMinZ = -(r + 1);
        float rowMaxZ = -r;
        float overlapZ = fmaxf(0.0f, fminf(shadowMaxZ, rowMaxZ) - fmaxf(shadowMinZ, rowMinZ));
        if (overlapZ <= 0.0f) continue;

        for (int c = colCentro - 1; c <= colCentro + 1; c++) {
            if (c < 0 || c >= 5) continue;
            if (getMapValue(r, c) != 1) continue;

            float blockMinX = c * blockWidth;
            float blockMaxX = (c + 1) * blockWidth;
            float overlapMinX = fmaxf(shadowMinX, blockMinX);
            float overlapMaxX = fminf(shadowMaxX, blockMaxX);
            float overlapX = fmaxf(0.0f, overlapMaxX - overlapMinX);
            if (overlapX <= 0.0f) continue;

            float overlapMinXLeft = overlapMinX;
            float overlapMaxXLeft = fminf(overlapMaxX, cameraX);
            float overlapLeftX = fmaxf(0.0f, overlapMaxXLeft - overlapMinXLeft);
            float overlapRightX = overlapX - overlapLeftX;

            float blockOverlapArea = overlapX * overlapZ;
            insideArea += blockOverlapArea;
            insideLeftArea += overlapLeftX * overlapZ;
            insideRightArea += overlapRightX * overlapZ;
        }
    }

    platformCoverage = insideArea / totalArea;
    platformCoverageLeft = insideLeftArea / (totalArea * 0.5f);
    platformCoverageRight = insideRightArea / (totalArea * 0.5f);

    if (platformCoverage < 0.0f) platformCoverage = 0.0f;
    if (platformCoverage > 1.0f) platformCoverage = 1.0f;
    if (platformCoverageLeft < 0.0f) platformCoverageLeft = 0.0f;
    if (platformCoverageLeft > 1.0f) platformCoverageLeft = 1.0f;
    if (platformCoverageRight < 0.0f) platformCoverageRight = 0.0f;
    if (platformCoverageRight > 1.0f) platformCoverageRight = 1.0f;
}

const float minPlatformCoverageToStand = 0.21f; 

typedef enum {
    CONTROL_MODE_WASD,
    CONTROL_MODE_ARROW,
    CONTROL_MODE_MOUSE,
    CONTROL_MODE_COUNT
} ControlMode;

static ControlMode getSelectedControlMode(void) {
    const char* controlValue = getConfigValue("control");
    if (!controlValue) {
        return CONTROL_MODE_WASD;
    }
    if (strcmp(controlValue, "Arrow") == 0) {
        return CONTROL_MODE_ARROW;
    }
    if (strcmp(controlValue, "Mouse") == 0) {
        return CONTROL_MODE_MOUSE;
    }
    return CONTROL_MODE_WASD;
}

static bool isMouseControlAllowed(void) {
    return getSelectedControlMode() == CONTROL_MODE_MOUSE;
}

static bool isArrowControlAllowed(void) {
    return getSelectedControlMode() == CONTROL_MODE_ARROW;
}

static bool isWASDControlAllowed(void) {
    return getSelectedControlMode() == CONTROL_MODE_WASD;
}

void resetPlayerPosition() {
    cameraX = 1.5f;
    cameraZ = 1.0f;
    cameraY = 1.2f;
    speed = 0.14f;
    lateralSpeed = 0.0f;
    jumpSpeed = 0.0f;
    deathFallSpeed = 0.0f;
    deathBallYOffset = 0.0f;
    isJumping = false;
    canAscend = false;
    isDead = false;
    deathTimer = 0;
    deadControlLocked = false;
    jumpBufferTimer = 0;
    jumpTimer = 0;
    platformCoverage = 0.0f;
    platformCoverageLeft = 0.0f;
    platformCoverageRight = 0.0f;
    mouseXPercent = 0.0f;
    mouseSpecialDown = false;
    isVictory = false;
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
        specialKeyStates[i] = false;
    }
}

static bool canAdvanceToNextLevel(void) {
    return getSelectedLevel() < getMaxLevelPreset();
}

static void endLevelConfirmSelection(void) {
    if (endLevelMenuIndex == 0) {
        if (isVictory) {
            if (canAdvanceToNextLevel()) {
                int proximoNivel = getSelectedLevel() + 1;
                setSelectedLevel(proximoNivel);
                setUnlockedLevel(proximoNivel);
                resetPlayerPosition();
            } else {
                currentScreen = Inicial;
            }
        } else {
            endLevelScreen = false;
        }
    } else if (endLevelMenuIndex == 1) {
        resetPlayerPosition();
        endLevelScreen = false; 
    } else {
        currentScreen = Inicial;
        endLevelScreen = false;
    }
    endLevelMenuIndex = 0;
    isVictory = false;
}

int isEndLevelScreenActive(void) {
    return endLevelScreen ? 1 : 0;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glLoadIdentity();
    gluPerspective(30.0, 1.33, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glutSetCursor(GLUT_CURSOR_INHERIT);
    initCustomFont("GAMERIA.ttf", 128.0f); 
    loadObjectFile("trophy.obj");

    initSaveSystem();
    const char* valStr = getConfigValue("p");
    if (valStr) {
        int loadedSkin = atoi(valStr);
        if (loadedSkin >= 1 && loadedSkin <= 4) {
            skinPlayer = loadedSkin;
        }
    }

    gerarTexturaSombra();
}

static void clearControlStates(void) {
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
        specialKeyStates[i] = false;
    }
    mouseSpecialDown = false;
}

void specialDown(int key, int x, int y) {
    if (currentScreen == Level) {
        if (endLevelScreen) {
            if (key == GLUT_KEY_UP) {
                endLevelMenuIndex = (endLevelMenuIndex + endLevelMenuCount - 1) % endLevelMenuCount;
            } else if (key == GLUT_KEY_DOWN) {
                endLevelMenuIndex = (endLevelMenuIndex + 1) % endLevelMenuCount;
            }
            glutPostRedisplay();
            return;
        }
        if (key < 256 && isArrowControlAllowed()) {
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
                specialKeyStates[key] = true;
            }
        }
        
        return;
        
    }

    if (key == GLUT_KEY_UP) {
        menuNavigateUp();
    } else if (key == GLUT_KEY_DOWN) {
        menuNavigateDown();
    } else if (key == GLUT_KEY_LEFT) {
        menuChangeStateLeft();
    } else if (key == GLUT_KEY_RIGHT) {
        menuChangeStateRight();
    }
    glutPostRedisplay();
}

void specialUp(int key, int x, int y) {
    if (currentScreen != Level) return;
    if (key < 256 && isArrowControlAllowed()) {
        specialKeyStates[key] = false;
    }
}

void keyboardDown(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;
    if (key == 'm') {
        if (getSelectedControlMode() == CONTROL_MODE_MOUSE) {
            return;
        }
        mouseControlEnabled = !mouseControlEnabled;
        if (!mouseControlEnabled) {
            mouseSpecialDown = false;
        }
        return;
    }

    if (key == 'r') {
        resetPlayerPosition();
        return;
    }

    if (currentScreen != Level) {
        if (currentScreen == Inicial) {
            if (key == 's') {
                currentScreen = SelecaoLevel;
                clearControlStates();
                return;
            }
            if (key == 'c') {
                currentScreen = Configuracoes;
                clearControlStates();
                return;
            }
            if (key == 'p') {
                currentScreen = Level;
                mouseControlEnabled = (getSelectedControlMode() == CONTROL_MODE_MOUSE);
                mouseSpecialDown = false;
                clearControlStates();
                return;
            }
            if (key == '\r') {
                menuSelectOption();
                mouseControlEnabled = (getSelectedControlMode() == CONTROL_MODE_MOUSE);
                mouseSpecialDown = false;
                clearControlStates();
                return;
            }
        } else if (currentScreen == Configuracoes) {
            if (key == 'b') {
                currentScreen = Inicial;
                clearControlStates();
                return;
            }
            if (key == '\r') {
                menuSelectOption();
                clearControlStates();
                return;
            }
        } else if (currentScreen == SelecaoLevel) {
            if (key == '\r') {
                menuSelectOption();
                mouseControlEnabled = (getSelectedControlMode() == CONTROL_MODE_MOUSE);
                mouseSpecialDown = false;
                clearControlStates();
                return;
            }
            if (key == 'b') {
                currentScreen = Inicial;
                clearControlStates();
                return;
            }
        }
        clearControlStates();
        return;
    }

        if (key == '1') {
            if (!isVictory) { 
                endLevelScreen = !endLevelScreen;
                if (endLevelScreen) {
                    endLevelMenuIndex = 0;
                }
                glutPostRedisplay();
            }
            return;
        }

if (key == '\r') {
    if (endLevelScreen) {
        int escolha = endLevelMenuIndex;

        endLevelScreen = false;
        
        if (isVictory && escolha == 0) {
            isVictory = false;
            
            if (canAdvanceToNextLevel()) {
                int proximoNivel = getSelectedLevel() + 1;
                setSelectedLevel(proximoNivel);
                setUnlockedLevel(proximoNivel);
                resetPlayerPosition();
            } else {
                currentScreen = Inicial;
            }
        } else {
            isVictory = false;
            endLevelMenuIndex = escolha;
            endLevelConfirmSelection();
        }
    } else {
        endLevelScreen = true;
        endLevelMenuIndex = 0;
    }
    
    glutPostRedisplay();
    return;
}

        ControlMode mode = getSelectedControlMode();
    if (key < 256 && (key == ' ' || key == 'j')) {
        keyStates[key] = true;
        jumpBufferTimer = JUMP_BUFFER_MAX;
    } else if (mode == CONTROL_MODE_WASD && key < 256) {
        keyStates[key] = true;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (currentScreen != Level) return;
    if (key >= 'A' && key <= 'Z') key += 32;
    if (key < 256 && (key == ' ' || key == 'j')) {
        keyStates[key] = false;
    } else if (getSelectedControlMode() == CONTROL_MODE_WASD && key < 256) {
        keyStates[key] = false;
    }
}

void mouseMotion(int x, int y) {
    if (!mouseControlEnabled || !isMouseControlAllowed()) return;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    if (width <= 0) return;

    float centerX = width * 0.5f;
    mouseXPercent = (x - centerX) / centerX;
    if (mouseXPercent < -1.0f) mouseXPercent = -1.0f;
    if (mouseXPercent > 1.0f) mouseXPercent = 1.0f;
}

void mouseClick(int button, int state, int x, int y) {
    if (!mouseControlEnabled || !isMouseControlAllowed()) return;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseSpecialDown = true;
        } else if (state == GLUT_UP) {
            mouseSpecialDown = false;
        }
    }
}

void drawHUD() {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glOrtho(0, 800, 600, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    char levelText[32];
    sprintf(levelText, "Level  %d", getSelectedLevel());
    
    glColor3f(0.0f, 0.0f, 0.0f); 
    drawHDTextScaled(19.0f, 25.0f, levelText, 0.22f);

    int levelEnd = getLevelEnd();
    if (levelEnd <= 0) levelEnd = 1;
    float progress = (-cameraZ) / (float)levelEnd;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    float barWidth = 220.0f;
    float barHeight = 24.0f;
    float margin = 20.0f;
    float outerLeft = 800.0f - margin - barWidth;
    float outerRight = 800.0f - margin;
    float outerTop = margin;
    float outerBottom = outerTop + barHeight;

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(outerLeft, outerTop);
        glVertex2f(outerRight, outerTop);
        glVertex2f(outerRight, outerBottom);
        glVertex2f(outerLeft, outerBottom);
    glEnd();

    float innerPadding = 4.0f;
    float innerLeft = outerLeft + innerPadding;
    float innerRight = outerRight - innerPadding;
    float innerTop = outerTop + innerPadding;
    float innerBottom = outerBottom - innerPadding;
    float fillWidth = (innerRight - innerLeft) * progress;

    glColor3f(0.8f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
        glVertex2f(innerLeft, innerTop);
        glVertex2f(innerLeft + fillWidth, innerTop);
        glVertex2f(innerLeft + fillWidth, innerBottom);
        glVertex2f(innerLeft, innerBottom);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    
    if (endLevelScreen) {
        int winW = glutGet(GLUT_WINDOW_WIDTH);
        int winH = glutGet(GLUT_WINDOW_HEIGHT);
        if (winW <= 0) winW = 800; 
        if (winH <= 0) winH = 600;

        float boxWidth = 500.0f;  
        float boxHeight = 320.0f; 
        
        float boxX = (winW - boxWidth) * 0.5f;
        float boxY = (winH - boxHeight) * 0.5f;
        float optionSpacing = 36.0f;

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, winW, winH, 0, -1, 1); 
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity(); 

        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
            glVertex2f(boxX, boxY);
            glVertex2f(boxX + boxWidth, boxY);
            glVertex2f(boxX + boxWidth, boxY + boxHeight);
            glVertex2f(boxX, boxY + boxHeight);
        glEnd();
        
        glColor3f(0.9f, 0.9f, 0.9f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(boxX, boxY);
            glVertex2f(boxX + boxWidth, boxY);
            glVertex2f(boxX + boxWidth, boxY + boxHeight);
            glVertex2f(boxX, boxY + boxHeight);
        glEnd();

        const char* titleText = isVictory ? "Nivel Concluido" : "Pausa";
        const char* optionText[3];
        if (isVictory) {
            optionText[0] = "Proximo Nivel";
            optionText[1] = "Jogar Novamente";
            optionText[2] = "Menu Inicial";
        } else {
            optionText[0] = "Continuar";
            optionText[1] = "Reiniciar";
            optionText[2] = "Menu Inicial";
        }
        
        float titleScale = 0.28f;
        float titleX = (winW - (getTextWidthPixels(titleText) * titleScale)) * 0.5f;
        float titleY = boxY + 50.0f; 
        
        if (isVictory) glColor3f(0.2f, 0.9f, 0.2f);
        else glColor3f(1.0f, 1.0f, 1.0f);
        
        drawHDTextScaled(titleX, titleY, titleText, titleScale);

        float optionScale = 0.22f;
        for (int i = 0; i < endLevelMenuCount; i++) {
            float optionY = titleY + 55.0f + (i * optionSpacing);
            
            char optionLine[128];
            if (i == endLevelMenuIndex) {
                sprintf(optionLine, "x %s", optionText[i]);
            } else {
                sprintf(optionLine, "  %s", optionText[i]);
            }
            
            float optionX = (winW - (getTextWidthPixels(optionLine) * optionScale)) * 0.5f;
            glColor3f(1.0f, 1.0f, 1.0f);
            drawHDTextScaled(optionX, optionY, optionLine, optionScale);
        }

        const char* footerText = "Pressione Enter para confirmar";
        float footerScale = 0.18f;
        float footerX = (winW - (getTextWidthPixels(footerText) * footerScale)) * 0.5f;
        float footerY = boxY + boxHeight - 30.0f;
        
        drawHDTextScaled(footerX, footerY, footerText, footerScale);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glLineWidth(1.0f);
        glPopAttrib();
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void timer(int v) {
    bool steerLeft = isMouseControlAllowed() && mouseControlEnabled && mouseXPercent < -0.10f;
    bool steerRight = isMouseControlAllowed() && mouseControlEnabled && mouseXPercent > 0.10f;
    bool mouseSteerActive = false;

    if (steerLeft || steerRight) {
        float absPercent = fabsf(mouseXPercent);
        if (absPercent > 0.70f) {
            mouseSteerActive = true;
        } else if (absPercent > 0.40f) {
            mouseSteerActive = (mouseSteerFrame % 2) == 0;
        } else {
            mouseSteerActive = (mouseSteerFrame % 3) == 0;
        }
        mouseSteerFrame++;
    } else {
        mouseSteerFrame = 0;
    }

    if (currentScreen != Level) {
        clearControlStates();
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    int levelEnd = getLevelEnd();
    if (!isDead && !endLevelScreen && (-cameraZ) >= (float)levelEnd) {
        endLevelScreen = true;
        isVictory = true;
        endLevelMenuIndex = 0;
        speed = 0.0f;
        lateralSpeed = 0.0f;
    }

    if (endLevelScreen) {
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    bool leftInput = false;
    bool rightInput = false;
    bool forwardInput = false;
    ControlMode mode = getSelectedControlMode();

    if (mode == CONTROL_MODE_ARROW) {
        leftInput = specialKeyStates[GLUT_KEY_LEFT];
        rightInput = specialKeyStates[GLUT_KEY_RIGHT];
        forwardInput = specialKeyStates[GLUT_KEY_UP];
    } else if (mode == CONTROL_MODE_WASD) {
        leftInput = keyStates['a'];
        rightInput = keyStates['d'];
        forwardInput = keyStates['w'];
    } else if (mode == CONTROL_MODE_MOUSE) {
        if (mouseSteerActive) {
            if (steerLeft) leftInput = true;
            if (steerRight) rightInput = true;
        }
        forwardInput = mouseSpecialDown;
    }

    if (isDead && !deadControlLocked) {
        float playerYPos = cameraY - 1.0f + deathBallYOffset;
        if (playerYPos < -0.8f) { 
            deadControlLocked = true;
        }
    }

    if (deadControlLocked) {
        leftInput = false;
        rightInput = false;
    }

    if (leftInput && !rightInput) {
        lateralSpeed -= accelLateral;
    } else if (rightInput && !leftInput) {
        lateralSpeed += accelLateral;
    } else {
        lateralSpeed *= (isDead ? 0.91f : 0.9f);
        if (fabsf(lateralSpeed) < 0.0005f) lateralSpeed = 0.0f;
    }

    if (lateralSpeed > maxLateralSpeed) lateralSpeed = maxLateralSpeed;
    if (lateralSpeed < -maxLateralSpeed) lateralSpeed = -maxLateralSpeed;

    if (isDead) {
        updatePlatformCoverage();
        float cameraXAntes = cameraX;
        cameraX += lateralSpeed;
        updatePlatformCoverage();

        if (platformCoverage > minPlatformCoverageToStand+0.05f) {
            cameraX = cameraXAntes;
            lateralSpeed = 0.0f;
            updatePlatformCoverage();
        }
    } else {
        cameraX += lateralSpeed;
    }

    if (!isDead) {
        if (forwardInput) {
            speed += accelVertical;
            if (speed > maxSpeed) speed = maxSpeed;
        } else {
            if (speed > minSpeed) speed -= dragVertical;
        }
    } else {
        speed = pow(speed, 1.009f); 
        if (speed < 0.005f) speed = 0.0f;
    }

    updatePlatformCoverage();
    bool temChaoEmbaixo = (platformCoverage > minPlatformCoverageToStand);

    if (isDead) {
        float shadowZPos = cameraZ + 3.65f;
        int colCentro = (int)floorf(cameraX / blockWidth);
        int rowCentro = (int)floorf(-shadowZPos);
        int rowFrente = rowCentro + 1;

        if (colCentro >= 0 && colCentro < 5 && getMapValue(rowFrente, colCentro) == 1) {
            speed = 0.0f;
        }
    }

    if (!isDead && !isJumping && !temChaoEmbaixo && cameraY <= groundY) {
        isDead = true;
        deathFallSpeed = 0.0f;
    }

    if (!isDead && jumpBufferTimer > 0 && !isJumping && cameraY <= groundY && temChaoEmbaixo) {
        isJumping = true;
        canAscend = true;
        jumpTimer = 0;
        jumpSpeed = 0.03f;
        jumpBufferTimer = 0; 
    }

    if (jumpBufferTimer > 0) jumpBufferTimer--;

    if (isDead) {
        deathFallSpeed -= gravity;
        deathBallYOffset += deathFallSpeed;
        deathTimer++;
        if (deathTimer >= 62) {
            resetPlayerPosition();
            deathTimer = 0;
        }
    } else if (isJumping) {
        jumpTimer++;
        bool jumpKeyPressed = (keyStates[' '] || keyStates['j']);

        if (!jumpKeyPressed && jumpTimer >= minJumpFrames) {
            canAscend = false;
        }

        if (canAscend && jumpTimer < maxJumpFrames && (jumpKeyPressed || jumpTimer < minJumpFrames)) {
            jumpSpeed += jumpAscendForce;
            if (jumpSpeed > maxJumpSpeed) jumpSpeed = maxJumpSpeed;
        } else {
            canAscend = false;
        }

        jumpSpeed -= gravity;
        cameraY += jumpSpeed;

        if (cameraY <= groundY) {
            cameraY = groundY;
            jumpSpeed = 0.0f;
            isJumping = false;
            jumpTimer = 0;
        }
    } else {
        if (temChaoEmbaixo) {
            cameraY = groundY;
        }
    }

    cameraZ -= speed;
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
