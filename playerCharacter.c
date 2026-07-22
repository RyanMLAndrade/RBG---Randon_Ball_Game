#include <GL/glut.h>
#include <math.h>
#include "playerCharacter.h"
#include "mapGenerator.h"
#include "constants.h"

extern GLuint texturaSombraID;
extern float deathBallYOffset;
extern bool isDead;
extern int skinPlayer; // Variável externa para controlar a skin
//extern float edgeTiltAngley; trashed, tirar dps

void drawFut(float raioEsfera) {
    float PHI = 1.618033988749895f;
    float PI = 3.141592653589793f;
    float vertices[12][3] = {
        {0, 1, PHI}, {0, -1, PHI}, {0, 1, -PHI}, {0, -1, -PHI},
        {1, PHI, 0}, {-1, PHI, 0}, {1, -PHI, 0}, {-1, -PHI, 0},
        {PHI, 0, 1}, {PHI, 0, -1}, {-PHI, 0, 1}, {-PHI, 0, -1}
    };
    
    float raioPrisma = raioEsfera * 0.35f; 
    float alturaPrisma = raioEsfera * 1.5f; 

    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);


    glDisable(GL_STENCIL_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(raioEsfera, 40, 40);
    
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); 
    glClear(GL_STENCIL_BUFFER_BIT); 

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for (int v = 0; v < 12; v++) {
        glPushMatrix(); 
        float vx = vertices[v][0], vy = vertices[v][1], vz = vertices[v][2];
        float longitude = atan2(vy, vx) * 180.0f / PI;
        float latitude = acos(vz / sqrt(vx*vx + vy*vy + vz*vz)) * 180.0f / PI;

        // Rotaciona na direção do vértice do treco
        glRotatef(longitude, 0.0f, 0.0f, 1.0f);
        glRotatef(latitude, 0.0f, 1.0f, 0.0f);
        
        glTranslatef(0.0f, 0.0f, -raioEsfera * 0.5f);

        float px[5], py[5];
        for (int i = 0; i < 5; i++) {
            float a = i * 2.0f * PI / 5.0f;
            px[i] = raioPrisma * cos(a);
            py[i] = raioPrisma * sin(a);
        }

        // Desenha as faces do prisma projetor
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0.0f, 0.0f, alturaPrisma);
            for (int i = 0; i <= 5; i++) glVertex3f(px[i % 5], py[i % 5], alturaPrisma);
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0.0f, 0.0f, 0.0f);
            for (int i = 5; i >= 0; i--) glVertex3f(px[i % 5], py[i % 5], 0.0f);
        glEnd();

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 5; i++) {
            glVertex3f(px[i % 5], py[i % 5], alturaPrisma);
            glVertex3f(px[i % 5], py[i % 5], 0.0f);
        }
        glEnd();
        glPopMatrix(); 
    }

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE); 
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glColor3f(0.0f, 0.0f, 0.0f);
    // Offset mínimo para evitar Z-Fighting
    glutSolidSphere(raioEsfera + 0.0005f, 40, 40); 

    glPopAttrib(); 
    glPopMatrix();
}

void drawPlayer() {
    static float rollRotation = 0.0f;
    static float tiltAnglex = 0.0f;
    static float tiltAngley = 0.0f;
    
    extern int isEndLevelScreenActive(void);
    if (!isEndLevelScreenActive()) {
        rollRotation -= (speed * 100.0f);
        if (rollRotation < -360.0f) rollRotation += 360.0f;
    }
    
    tiltAnglex = lateralSpeed * -250.0f; 
    tiltAngley = lateralSpeed * -400.0f;
    //tiltAngley += edgeTiltAngley; trashed
    
    float height = cameraY - groundY;

    float shadowZPos = cameraZ + 3.65f; 
    float radius = 0.21f + (height * 0.05f);
    if (radius < 0.01f) radius = 0.01f;

    int playerRow = (int)floorf(-shadowZPos);

    int colCentro = (int)floorf(cameraX / blockWidth);
    int colEsquerda = colCentro - 1;
    int colDireita = colCentro + 1;

    if (!isDead) {
        glPushMatrix();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturaSombraID);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            for (int c = colEsquerda; c <= colDireita; c++) {
                if (c < 0 || c >= 5) continue;
                if (getMapValue(playerRow, c) != 1) continue;

                float limiteEsquerdoBloco = c * blockWidth;
                float limiteDireitoBloco = (c + 1) * blockWidth;

                float sombraMinX = cameraX - radius;
                float sombraMaxX = cameraX + radius;
                float sombraMinZ = shadowZPos - radius;
                float sombraMaxZ = shadowZPos + radius;

                if (sombraMinX < limiteEsquerdoBloco) sombraMinX = limiteEsquerdoBloco;
                if (sombraMaxX > limiteDireitoBloco)  sombraMaxX = limiteDireitoBloco;

                if (sombraMinX < sombraMaxX) {
                    glBegin(GL_QUADS);
                        float uMin = (sombraMinX - (cameraX - radius)) / (radius * 2.0f);
                        float uMax = (sombraMaxX - (cameraX - radius)) / (radius * 2.0f);

                        glTexCoord2f(uMin, 0.0f); glVertex3f(sombraMinX, 0.01f, sombraMinZ);
                        glTexCoord2f(uMax, 0.0f); glVertex3f(sombraMaxX, 0.01f, sombraMinZ);
                        glTexCoord2f(uMax, 1.0f); glVertex3f(sombraMaxX, 0.01f, sombraMaxZ);
                        glTexCoord2f(uMin, 1.0f); glVertex3f(sombraMinX, 0.01f, sombraMaxZ);
                    glEnd();
                }
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        glPopMatrix();
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glTranslatef(cameraX, cameraY - 1.0f + deathBallYOffset, cameraZ + 4.0f);
        float divisor = (12 * speed * speed * 4);
        if (divisor < 1) divisor = 1; 
        glRotatef(tiltAngley / divisor, 0.0f, 0.0f, 1.0f);
        glRotatef(tiltAnglex, 0.0f, 1.0f, 0.0f);
        glRotatef(rollRotation, 1.0f, 0.0f, 0.0f);
        
        if (skinPlayer == 1) {
            // Desenha a esfera laranja
            glColor3f(1.0f, 0.8f, 0.3f);
            glutSolidSphere(0.1f, 20, 20);
            glColor3f(0.0f, 0.0f, 0.0f); 
            glutWireSphere(0.105f, 10, 10);
        } else if (skinPlayer == 2) {
            // Desenha a esfera rosa
            glColor3f(1.0f, 0.4f, 0.6f);
            glutSolidSphere(0.1f, 20, 20);
            glColor3f(0.3f, 0.0f, 0.0f); 
            glutWireSphere(0.105f, 16, 6);
        } else if (skinPlayer == 3) {
            drawFut(0.1f); 
        } else if (skinPlayer == 4) {
            // Desenha a esfera diamante
            glColor3f(0.0f, 0.7f, 0.7f);
            glutSolidSphere(0.1f, 4, 4);
            glColor3f(0.5f, 1.0f, 1.0f); 
            glutWireSphere(0.107f, 12, 10);
        } else {
            // Desenha a esfera preta transparente
            glColor3f(0.0f, 0.0f, 0.0f);
            glutSolidSphere(0.1f, 6, 4);
            glColor3f(0.5f, 0.5f, 0.5f); 
            glutWireSphere(0.107f, 8, 4);
        }
        
    glPopMatrix();
    glPopAttrib();
}
