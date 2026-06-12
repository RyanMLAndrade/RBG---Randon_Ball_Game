#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include "mapGenerator.h"
#include "constants.h"

// Gera um valor aleatório a cada frame
float getNoise(int x, int seed) {
    int n = x + seed * 57;
    n = (n << 13) ^ n;
    int res = (n * (n * n * 15131 + 789222) + 137631258) & 0x7fffffff;
    return (float)res / 2147483647.0f;
}

float interpolate(float a, float b, float x) {
    float ft = x * 3.1415927f;
    float f = (1.0f - cos(ft)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

float perlin1D(float x, int seed) {
    int intX = (int)floor(x);
    float fracX = x - (float)intX;
    float v1 = getNoise(intX, seed);
    float v2 = getNoise(intX + 1, seed);
    return interpolate(v1, v2, fracX);
}

int getMapValue(int index, int col) {
    // Evita ler índices negativos de segurança na largada
    if (index < 0) return 1;
    if (index < 7) return 1;
    
    int seed = 0;
    float frequency = 0.15f;
    float difficultyFactor = (index / 1500.0f);
    if (difficultyFactor > 0.45f) difficultyFactor = 0.45f;
    float threshold = 0.35f + difficultyFactor;

    float noiseValue = perlin1D(index * frequency, seed + (col * 1000));
    return (noiseValue > threshold) ? 1 : 0;
} 

void drawCube(float x, float z, int i, int col) {
    float yTop = 0.0f;
    float yBottom = -0.6f;
    float xRight = x + blockWidth;
    
    // CORREÇÃO DA ORIENTAÇÃO DO CUBO:
    // Como z agora entra negativo (ex: -10.0f), para o bloco se estender para o "fundo" da tela,
    // a face frontal fica em z (ex: -10.0f) e a face traseira fica em z - 1.0f (ex: -11.0f)
    float zFront = z;
    float zBack = z - 1.0f;

    glBegin(GL_QUADS);
        // face de cima (verde)
        if ((i + col) % 2 == 0) 
            glColor3f(0.2f, 0.7f, 0.2f); // Verde claro
        else 
            glColor3f(0.18f, 0.6f, 0.18f); // Verde escuro
            
        glVertex3f(x, yTop, zBack);
        glVertex3f(xRight, yTop, zBack);
        glVertex3f(xRight, yTop, zFront);
        glVertex3f(x, yTop, zFront);

        // lateral - areia
        glColor3f(0.85f, 0.75f, 0.45f); 

        // Lateral Direita
        glVertex3f(xRight, yTop, zBack);
        glVertex3f(xRight, yTop, zFront);
        glVertex3f(xRight, yBottom, zFront);
        glVertex3f(xRight, yBottom, zBack);

        // Lateral Esquerda
        glVertex3f(x, yTop, zBack);
        glVertex3f(x, yTop, zFront);
        glVertex3f(x, yBottom, zFront);
        glVertex3f(x, yBottom, zBack);

        // Face Frontal
        glVertex3f(x, yTop, zFront);
        glVertex3f(xRight, yTop, zFront);
        glVertex3f(xRight, yBottom, zFront);
        glVertex3f(x, yBottom, zFront);

    glEnd();
}

void renderMap() {
    int currentIdx = (int)(-cameraZ);
    
    // Garante que o laço inicial não tente ler índices negativos de matriz
    int startIdx = currentIdx - 7;
    //if (startIdx < 0) startIdx = 0;

    for (int i = startIdx; i < currentIdx + 90; i++) {
        for (int col = 0; col < 5; col++) {
            if (getMapValue(i, col) == 1) {
                // CORREÇÃO CRÍTICA: Passamos "-i" para posicionar o bloco corretamente 
                // no sentido de avanço do Z negativo do OpenGL
                drawCube(col * blockWidth, -i, i, col);
            }
        }
    }
}
