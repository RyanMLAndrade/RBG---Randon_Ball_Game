#include <GL/glut.h>
#include <math.h>
#include "playerCharacter.h"
#include "mapGenerator.h"
#include "constants.h"

extern GLuint texturaSombraID;
extern float deathBallYOffset;
extern bool isDead;

void drawPlayer() {
    static float rollRotation = 0.0f;
    static float tiltAnglex = 0.0f;
    static float tiltAngley = 0.0f;
    
    // Roll sempre atualiza (ilusão de movimento para frente)
    rollRotation -= (speed * 100.0f);
    if (rollRotation < -360.0f) rollRotation += 360.0f;
    
    // Tilt (orientação lateral) continua atualizando mesmo após a morte
    tiltAnglex = lateralSpeed * -250.0f; 
    tiltAngley = lateralSpeed * -400.0f;
    
    float height = cameraY - groundY;

    // --- LOGICA DE SOMBRA PROJETADA EM MÚLTIPLOS BLOCOS ---
    float shadowZPos = cameraZ + 3.65f; 
    float radius = 0.21f + (height * 0.05f);
    if (radius < 0.01f) radius = 0.01f;

    // Índices de linhas do mapa baseado no Z atual
    int playerRow = (int)floorf(-shadowZPos);

    // Descobre a coluna central, a da esquerda e a da direita que a sombra pode tocar
    int colCentro = (int)floorf(cameraX / blockWidth);
    int colEsquerda = colCentro - 1;
    int colDireita = colCentro + 1;

    if (!isDead) {
        // Ativa as configurações de renderização da sombra uma única vez
        glPushMatrix();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturaSombraID);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f); 

            // Vamos rodar um laço pelas 3 colunas que a sombra potencialmente intersecta
            for (int c = colEsquerda; c <= colDireita; c++) {
                // Se a coluna estiver fora dos limites da pista (0 a 4), ignora
                if (c < 0 || c >= 5) continue;

                // CHECAGEM REAL DO MAPA: Se o bloco dessa coluna específica for um buraco (0),
                // a sombra NÃO será desenhada aqui.
                if (getMapValue(playerRow, c) != 1) continue;

                // Calcula os limites físicos DESTE bloco específico que estamos avaliando no loop
                float limiteEsquerdoBloco = c * blockWidth;
                float limiteDireitoBloco = (c + 1) * blockWidth;

                // Reseta os limites do quadrado para o tamanho total da sombra a cada iteração
                float sombraMinX = cameraX - radius-1;
                float sombraMaxX = cameraX + radius;
                float sombraMinZ = shadowZPos - radius;
                float sombraMaxZ = shadowZPos + radius;

                // Recorta o quadrado da sombra para caber estritamente dentro DESTE bloco
                if (sombraMinX < limiteEsquerdoBloco) sombraMinX = limiteEsquerdoBloco;
                if (sombraMaxX > limiteDireitoBloco)  sombraMaxX = limiteDireitoBloco;

                // Se após o recorte, sobrou um pedaço válido de sombra dentro do bloco, desenha ele!
                if (sombraMinX < sombraMaxX) {
                    glBegin(GL_QUADS);
                        // Mapeia dinamicamente as coordenadas de textura (U) baseado no recorte
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

    // --- ESFERA DO PERSONAGEM ---
    glPushMatrix();
        glTranslatef(cameraX, cameraY - 1.0f + deathBallYOffset, cameraZ + 4.0f);
        float divisor = (12 * speed * speed * 4);
        if (divisor < 1) divisor = 1; 
        glRotatef(tiltAngley / divisor, 0.0f, 0.0f, 1.0f);
        glRotatef(tiltAnglex, 0.0f, 1.0f, 0.0f);
        glRotatef(rollRotation, 1.0f, 0.0f, 0.0f);
        glColor3f(1.0f, 0.5f, 0.0f);
        glutSolidSphere(0.1f, 20, 20);
        glColor3f(0.0f, 0.0f, 0.0f); 
        glutWireSphere(0.105f, 10, 10);
    glPopMatrix();
}
