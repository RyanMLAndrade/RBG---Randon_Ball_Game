#include <GL/glut.h>
#include <math.h>
#include "gameEngine.h"
#include "mapGenerator.h" // Importado para ler os dados da pista
#include "constants.h"

float cameraX = 1.5f, cameraZ = 1.0f, cameraY = 1.2f;
float speed = 0.1f, lateralSpeed = 0.0f;
float maxSpeed = 0.30f, minSpeed = 0.1f;
float accelVertical = 0.002f, dragVertical = 0.0005f;
float accelLateral = 0.003f, maxLateralSpeed = 0.055f;
float blockWidth = 0.6f;

static bool deadControlLocked = false;


// Física do Pulo
float jumpSpeed = 0.0f;
float gravity = 0.003f;
float groundY = 1.2f;
float jumpAscendForce = 0.007f; 
float maxJumpSpeed = 0.06f;
bool isJumping = false;
bool canAscend = false; 

// cayote jump
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

// ID da textura da sombra projetada
GLuint texturaSombraID;

// Estado do jogo
bool isDead = false;
static float deathFallSpeed = 0.0f;
float deathBallYOffset = 0.0f;
static float platformCoverage = 0.0f;
static float platformCoverageLeft = 0.0f;
static float platformCoverageRight = 0.0f;

// ============================================================================
// --- 2. SISTEMA DE TEXTURA DA SOMBRA E INICIALIZAÇÃO ---
// ============================================================================

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

            pixels[y][x][0] = 0;   // R
            pixels[y][x][1] = 0;   // G
            pixels[y][x][2] = 0;   // B
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

    for (int r = rowCentro - 1; r <= rowCentro + 1; r++) {
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

const float minPlatformCoverageToStand = 0.21f; // 21% coverage = 79% outside the block

static void resetPlayerPosition() {
    cameraX = 1.5f;
    cameraZ = 1.0f;
    cameraY = 1.2f;
    speed = 0.1f;
    lateralSpeed = 0.0f;
    jumpSpeed = 0.0f;
    deathFallSpeed = 0.0f;
    deathBallYOffset = 0.0f;
    isJumping = false;
    canAscend = false;
    isDead = false;
    deadControlLocked = false;
    jumpBufferTimer = 0;
    jumpTimer = 0;
    platformCoverage = 0.0f;
    platformCoverageLeft = 0.0f;
    platformCoverageRight = 0.0f;
    mouseXPercent = 0.0f;
    mouseSpecialDown = false;
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
        specialKeyStates[i] = false;
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, 1.33, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glutSetCursor(GLUT_CURSOR_INHERIT);

    // Gera a textura agora que as funções sabem o escopo das variáveis
    gerarTexturaSombra(); 
}

// ============================================================================
// --- 3. CONTROLES E ENTRADAS DE TECLADO ---
// ============================================================================

void specialDown(int key, int x, int y) { if (key < 256) specialKeyStates[key] = true; }
void specialUp(int key, int x, int y) { if (key < 256) specialKeyStates[key] = false; }

void keyboardDown(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;
    if (key == 'm') {
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

    if (key < 256) {
        keyStates[key] = true;
        if (key == ' ' || key == 'j') {
            jumpBufferTimer = JUMP_BUFFER_MAX; 
        }
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key >= 'A' && key <= 'Z') key += 32;
    if (key < 256) keyStates[key] = false;
}

void mouseMotion(int x, int y) {
    if (!mouseControlEnabled) return;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    if (width <= 0) return;

    float centerX = width * 0.5f;
    mouseXPercent = (x - centerX) / centerX;
    if (mouseXPercent < -1.0f) mouseXPercent = -1.0f;
    if (mouseXPercent > 1.0f) mouseXPercent = 1.0f;
}

void mouseClick(int button, int state, int x, int y) {
    if (!mouseControlEnabled) return;

    if (button == GLUT_MIDDLE_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseSpecialDown = true;
        } else if (state == GLUT_UP) {
            mouseSpecialDown = false;
        }
    }
}

//RENDERIZAÇÃO DA INTERFACE (HUD) E LOOP DE FÍSICA ---

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glOrtho(0, 800, 0, 600, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    float squareSize = 50.0f;
    float padding = 10.0f;
    float leftX = 20.0f;
    float rightX = leftX + squareSize + padding;
    float bottomY = 530.0f;
    float topY = 580.0f;

    glColor3f(0.3f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(leftX, bottomY);
        glVertex2f(leftX + squareSize, bottomY);
        glVertex2f(leftX + squareSize, topY);
        glVertex2f(leftX, topY);
    glEnd();

    float coverageWidth = squareSize * platformCoverage;
    if (coverageWidth < 0.0f) coverageWidth = 0.0f;
    if (coverageWidth > squareSize) coverageWidth = squareSize;

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(leftX, bottomY);
        glVertex2f(leftX + coverageWidth, bottomY);
        glVertex2f(leftX + coverageWidth, topY);
        glVertex2f(leftX, topY);
    glEnd();

    glColor3f(0.15f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(rightX, bottomY);
        glVertex2f(rightX + squareSize, bottomY);
        glVertex2f(rightX + squareSize, topY);
        glVertex2f(rightX, topY);
    glEnd();

    float halfSize = (squareSize - 4.0f) * 0.5f;
    float leftBar = halfSize * platformCoverageLeft;
    float rightBar = halfSize * platformCoverageRight;
    if (leftBar < 0.0f) leftBar = 0.0f;
    if (leftBar > halfSize) leftBar = halfSize;
    if (rightBar < 0.0f) rightBar = 0.0f;
    if (rightBar > halfSize) rightBar = halfSize;

    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(rightX + 2.0f, bottomY + 2.0f);
        glVertex2f(rightX + 2.0f + leftBar, bottomY + 2.0f);
        glVertex2f(rightX + 2.0f + leftBar, topY - 2.0f);
        glVertex2f(rightX + 2.0f, topY - 2.0f);
    glEnd();

    glColor3f(1.0f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
        glVertex2f(rightX + 2.0f + halfSize, bottomY + 2.0f);
        glVertex2f(rightX + 2.0f + halfSize + rightBar, bottomY + 2.0f);
        glVertex2f(rightX + 2.0f + halfSize + rightBar, topY - 2.0f);
        glVertex2f(rightX + 2.0f + halfSize, topY - 2.0f);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void timer(int v) {
    // Movimentação Lateral e Frontal 
    bool steerLeft = mouseControlEnabled && mouseXPercent < -0.10f;
    bool steerRight = mouseControlEnabled && mouseXPercent > 0.10f;
    bool mouseSteerActive = false;

    if (steerLeft || steerRight) {
        float absPercent = fabsf(mouseXPercent);
        if (absPercent > 0.70f) {
            // velocidade máxima
            mouseSteerActive = true;
        } else if (absPercent > 0.40f) {
            // 1/2: pressionado 1 frame, solto 1 frame
            mouseSteerActive = (mouseSteerFrame % 2) == 0;
        } else {
            // entre 10% e 40% -> 1/3: pressionado 1 frame, solto 2 frames
            mouseSteerActive = (mouseSteerFrame % 3) == 0;
        }
        mouseSteerFrame++;
    } else {
        mouseSteerFrame = 0;
    }

    bool leftInput = specialKeyStates[GLUT_KEY_LEFT] || keyStates['a'];
    bool rightInput = specialKeyStates[GLUT_KEY_RIGHT] || keyStates['d'];

    if (mouseSteerActive && steerLeft) leftInput = true;
    if (mouseSteerActive && steerRight) rightInput = true;

    // Verifica se o personagem desapareceu da tela quando morto
    if (isDead && !deadControlLocked) {
        float playerYPos = cameraY - 1.0f + deathBallYOffset;
        if (playerYPos < -0.8f) { // Personagem suficientemente abaixo
            deadControlLocked = true;
        }
    }

    // Bloqueia entrada ANTES de usar
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
        // A velocidade residual pode puxar o personagem um pouco para dentro do bloco,
        // mas até o limiar de minPlatformCoverageToStand. Se ultrapassar, rejeita o movimento.
        updatePlatformCoverage();
        float coverageAntes = platformCoverage;
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
        if (specialKeyStates[GLUT_KEY_UP] || keyStates['w'] || (mouseControlEnabled && mouseSpecialDown)) {
            speed += accelVertical;
            if (speed > maxSpeed) speed = maxSpeed;
        } else {
            if (speed > minSpeed) speed -= dragVertical;
        }
    } else {
        speed -= dragVertical * 1.5f;
        if (speed < 0.0f) speed = 0.0f;
    }

    updatePlatformCoverage();
    bool temChaoEmbaixo = (platformCoverage > minPlatformCoverageToStand);

    // Checagem de morte
    if (!isDead && !isJumping && !temChaoEmbaixo && cameraY <= groundY) {
        isDead = true;
        deathFallSpeed = 0.0f;
    }

    // Inicialização do pulo
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
