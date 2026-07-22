#include <GL/glut.h>
#include <math.h>
#include "background.h"
#include "constants.h"

static const float DISTANCIA_CEU = -80.0f;      // Parede do céu 
static const float DISTANCIA_FOG = -75.0f;      // Início da névoa 
static const float PROFUNDIDADE_MAR = -0.15f;   // Mar logo abaixo da pista 
static const float LARGURA_CENARIO = 200.0f;    // Extensão lateral
static const float ALTURA_CEU = 20.0f;          // Altura da parede 

//Função para desenhar uma oval (núvem)
void drawOval(float centerX, float centerY, float radiusX, float radiusY) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(centerX, centerY, DISTANCIA_CEU + 1.0f);
        for (int i = 0; i <= 20; i++) {
            float theta = 2.0f * 3.1415926f * (float)i / 20.0f;
            glVertex3f(centerX + (radiusX * cosf(theta)), centerY + (radiusY * sinf(theta)), DISTANCIA_CEU + 1.0f);
        }
    glEnd();
}

//desenha ceu núvens e montanhas
void drawBackground() {
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
        glTranslatef(cameraX, 0.0f, cameraZ);

        glDisable(GL_DEPTH_TEST);

        glBegin(GL_QUADS);
            // Degrade do mar
            glColor3f(0.0f, 0.1f, 0.3f); // Azul escuro
            glVertex3f(-LARGURA_CENARIO, PROFUNDIDADE_MAR, 20.0f);
            glVertex3f(LARGURA_CENARIO, PROFUNDIDADE_MAR, 20.0f);
            
            glColor3f(0.0f, 0.4f, 0.6f); // Azul menos escuro
            glVertex3f(LARGURA_CENARIO, PROFUNDIDADE_MAR, DISTANCIA_CEU);
            glVertex3f(-LARGURA_CENARIO, PROFUNDIDADE_MAR, DISTANCIA_CEU);

            // Degrade do ceu
            glColor3f(0.6f, 0.9f, 1.0f); // Base no horizonte
            glVertex3f(-LARGURA_CENARIO, PROFUNDIDADE_MAR, DISTANCIA_CEU);
            glVertex3f(LARGURA_CENARIO, PROFUNDIDADE_MAR, DISTANCIA_CEU);
            
            glColor3f(0.0f, 0.7f, 0.9f); // Topo da parede
            glVertex3f(LARGURA_CENARIO, ALTURA_CEU, DISTANCIA_CEU);
            glVertex3f(-LARGURA_CENARIO, ALTURA_CEU, DISTANCIA_CEU);
        glEnd();
    glPopMatrix();
    glPopAttrib();
}

static void drawMountainMesh(float zMontanha) {
    glBegin(GL_TRIANGLES);
        // primeira cadeia
        glColor3f(0.16f, 0.48f, 0.34f);
        glVertex3f(-32.0f, PROFUNDIDADE_MAR + 10.0f, zMontanha);
        glVertex3f(-28.0f, PROFUNDIDADE_MAR + 13.0f, zMontanha);
        glVertex3f(-24.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);

        glColor3f(0.14f, 0.44f, 0.32f);
        glVertex3f(-32.0f, PROFUNDIDADE_MAR + 10.0f, zMontanha);
        glVertex3f(-33.2004f, PROFUNDIDADE_MAR + 5.9987f, zMontanha);
        glVertex3f(-24.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);

        glColor3f(0.13f, 0.42f, 0.30f);
        glVertex3f(-33.2004f, PROFUNDIDADE_MAR + 5.9987f, zMontanha);
        glVertex3f(-13.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(-24.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);

        glColor3f(0.11f, 0.38f, 0.28f);
        glVertex3f(-13.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(-33.2004f, PROFUNDIDADE_MAR + 5.9987f, zMontanha);
        glVertex3f(-9.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);

        glColor3f(0.10f, 0.34f, 0.26f);
        glVertex3f(-33.2004f, PROFUNDIDADE_MAR + 5.9987f, zMontanha);
        glVertex3f(-35.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(-9.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);

        // segunda cadeia
        glColor3f(0.14f, 0.42f, 0.30f);
        glVertex3f(-6.0f, PROFUNDIDADE_MAR + 3.0f, zMontanha);
        glVertex3f(-9.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(-11.9999f, PROFUNDIDADE_MAR + 3.7499f, zMontanha);

        glColor3f(0.12f, 0.38f, 0.28f);
        glVertex3f(-1.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(0.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);
        glVertex3f(-6.0f, PROFUNDIDADE_MAR + 3.0f, zMontanha);

        glColor3f(0.10f, 0.34f, 0.26f);
        glVertex3f(-1.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(10.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);
        glVertex3f(0.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);

        glColor3f(0.09f, 0.32f, 0.24f);
        glVertex3f(0.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);
        glVertex3f(-9.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(-6.0f, PROFUNDIDADE_MAR + 3.0f, zMontanha);

        glColor3f(0.08f, 0.30f, 0.22f);
        glVertex3f(0.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);
        glVertex3f(5.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(10.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);

        glColor3f(0.07f, 0.28f, 0.20f);
        glVertex3f(5.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(-9.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(0.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);

        // terceira cadeia
        glColor3f(0.16f, 0.48f, 0.34f);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 12.0f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(31.0f, PROFUNDIDADE_MAR + 10.0f, zMontanha);

        glColor3f(0.14f, 0.44f, 0.32f);
        glVertex3f(23.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);
        glVertex3f(31.0f, PROFUNDIDADE_MAR + 10.0f, zMontanha);
        glVertex3f(28.0f, PROFUNDIDADE_MAR + 12.0f, zMontanha);

        glColor3f(0.13f, 0.42f, 0.30f);
        glVertex3f(23.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);
        glVertex3f(31.0f, PROFUNDIDADE_MAR + 10.0f, zMontanha);

        glColor3f(0.11f, 0.38f, 0.28f);
        glVertex3f(23.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);
        glVertex3f(24.0367f, PROFUNDIDADE_MAR + 4.4f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);

        glColor3f(0.10f, 0.34f, 0.26f);
        glVertex3f(23.0f, PROFUNDIDADE_MAR + 9.0f, zMontanha);
        glVertex3f(19.0f, PROFUNDIDADE_MAR + 8.0f, zMontanha);
        glVertex3f(24.0367f, PROFUNDIDADE_MAR + 4.4f, zMontanha);

        glColor3f(0.09f, 0.32f, 0.24f);
        glVertex3f(10.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);
        glVertex3f(20.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(5.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);

        glColor3f(0.08f, 0.30f, 0.22f);
        glVertex3f(15.0f, PROFUNDIDADE_MAR + 4.0f, zMontanha);
        glVertex3f(20.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(10.0f, PROFUNDIDADE_MAR + 2.0f, zMontanha);

        glColor3f(0.07f, 0.28f, 0.20f);
        glVertex3f(15.0f, PROFUNDIDADE_MAR + 4.0f, zMontanha);
        glVertex3f(20.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);

        glColor3f(0.06f, 0.26f, 0.18f);
        glVertex3f(20.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 0.0f, zMontanha);
        glVertex3f(35.0f, PROFUNDIDADE_MAR + 5.0f, zMontanha);

        glColor3f(0.05f, 0.24f, 0.16f);
        glVertex3f(19.0f, PROFUNDIDADE_MAR + 8.0f, zMontanha);
        glVertex3f(15.0f, PROFUNDIDADE_MAR + 4.0f, zMontanha);
        glVertex3f(24.0367f, PROFUNDIDADE_MAR + 4.4f, zMontanha);
    glEnd();
}

void drawMountain() {
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
        glTranslatef(cameraX, 0.0f, cameraZ);
        
        glDisable(GL_DEPTH_TEST);

        float zMontanha = DISTANCIA_CEU + 0.5f;

        drawMountainMesh(zMontanha);
    glPopMatrix();
    glPopAttrib();
}

void drawClouds() {
    static float animacaoX = 40.0f; // Aonde começa as núvens
    animacaoX += 0.01f; // velocidade núvens

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glPushMatrix();
        
        glTranslatef(cameraX, 0.0f, cameraZ);
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);

        float xmax = 95.0f; 

        drawOval(fmodf(-46.5f + animacaoX, xmax) - 50.0f, 14.0f, 7.75f, 1.5f);
        drawOval(fmodf(-40.0f + animacaoX, xmax) - 50.0f, 9.0f,  8.75f, 1.75f);
        drawOval(fmodf(-36.5f + animacaoX, xmax) - 50.0f, 11.5f, 3.75f, 0.75f);
        drawOval(fmodf(-28.0f + animacaoX, xmax) - 50.0f, 13.5f, 6.0f,  1.5f);
        drawOval(fmodf(-22.5f + animacaoX, xmax) - 50.0f, 10.5f, 3.0f,  0.6f);
        drawOval(fmodf(-13.5f  + animacaoX, xmax) - 50.0f, 16.5f, 11.25f, 2.25f);
        drawOval(fmodf(-5.0f   + animacaoX, xmax) - 50.0f, 10.0f, 10.0f, 2.0f); 
        drawOval(fmodf(8.5f    + animacaoX, xmax) - 50.0f, 16.0f, 7.5f,  1.5f);
        drawOval(fmodf(17.5f   + animacaoX, xmax) - 50.0f, 11.5f, 3.75f, 0.75f);
        drawOval(fmodf(25.0f   + animacaoX, xmax) - 50.0f, 15.0f, 6.25f, 1.25f);
        drawOval(fmodf(30.5f   + animacaoX, xmax) - 50.0f, 9.0f,  8.75f, 1.75f);
        drawOval(fmodf(34.5f   + animacaoX, xmax) - 50.0f, 12.5f, 3.75f, 0.75f);
    glPopMatrix();
    glPopAttrib();
}

void drawFog3D() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING); 

    float zFog = cameraZ + DISTANCIA_FOG; 
    
    // fog do mar
    glBegin(GL_POLYGON);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f); 
        glVertex3f(cameraX + 35.0f,  0.5f, zFog);
        glVertex3f(cameraX - 35.0f,  0.2f, zFog);
        
        glColor4f(1.0f, 1.0f, 1.0f, 0.0f); 
        glVertex3f(cameraX - 35.0f, -4.0f - 1.0f, zFog);
        glVertex3f(cameraX + 35.0f, -4.0f - 1.0f, zFog);
    glEnd();

    // fog do céu
    glBegin(GL_POLYGON);
        glColor4f(0.8f, 0.8f, 1.0f, 1.0f); 
        glVertex3f(cameraX + 35.0f,  -0.1f, zFog);
        glVertex3f(cameraX - 35.0f,  -0.1f, zFog);
        
        glColor4f(0.8f, 0.8f, 1.0f, 0.0f); 
        glVertex3f(cameraX - 35.0f, 4.0f - 1.0f, zFog);
        glVertex3f(cameraX + 35.0f, 4.0f - 1.0f, zFog);
    glEnd();
    
    // Reativa o que foi desativado para não quebrar o restante do jogo
    glEnable(GL_LIGHTING); 
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
