#include <GL/glut.h>
#include "gameEngine.h"
#include "background.h"
#include "mapGenerator.h"
#include "playerCharacter.h"
#include "uiScreens.h"
#include "readwrite.h"
#include "drawObject.h"
#include "constants.h"

extern GLuint texturaSombraID; 

void drawGoldenTrophyIsolated(float x, float y, float z) {
    // Salva o estado atual da iluminação e dos enabled bits
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);

    glPushMatrix();
        glTranslatef(x, y, z);
        
        static float levelTrophyRot = 0.0f;
        levelTrophyRot += 0.8f;
        glRotatef(levelTrophyRot, 0.0f, 1.0f, 0.0f);

        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE); 
        glDisable(GL_COLOR_MATERIAL); 

        // Material Dourado
        glColor3f(1.0f,1.0f,0.0f);
        GLfloat mat_ambient[]   = { 0.24725f*2, 0.1995f*2, 0.0745f*2, 1.0f };
        GLfloat mat_diffuse[]   = { 0.75164f*2, 0.60648f*2, 0.22648f*2, 1.0f };
        GLfloat mat_specular[]  = { 0.628281f, 0.555802f, 0.366065f, 1.0f };
        GLfloat shine           = 50.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, shine);

        glScalef(0.03f, 0.03f, 0.03f);
        
        drawObject();
    glPopMatrix();

    glPopAttrib();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Lógica do FOV
    float fovOriginal = 24.0f; 
    float fovAlvo = fovOriginal;
    if (speed > 0.1f) fovAlvo = fovOriginal + (speed * 23.0f); 
    if (fovAlvo > 65.0f) fovAlvo = 65.0f;

    static float fovTransicao = 30.0f; 
    float taxaSuavizacao = 0.02f; 
    fovTransicao += (fovAlvo - fovTransicao) * taxaSuavizacao;

    gluPerspective(fovTransicao, 1.33, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(cameraX, cameraY, 8.0 + cameraZ, 
              cameraX, cameraY - 0.8f, cameraZ - 2.0, 
              0.0, 1.0, 0.0);
    
    // Configuração de Iluminação global
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat direcaoSol[4], luzBranca[4], luzAmbiente[4];
    getCurrentLevelLighting(direcaoSol, luzBranca, luzBranca, luzAmbiente);

    glLightfv(GL_LIGHT0, GL_POSITION, direcaoSol);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzBranca);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzBranca);
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    
    int levelEnd = getLevelEnd();
    
    


    drawUIScreen(); 

    if (currentScreen == Level && (-cameraZ < levelEnd - 0.01f)) {
        float trophyX = 2.5f * blockWidth; 
        float trophyY = 1.0f; 
        float trophyZ = -(float)levelEnd - 0.5f; 

        drawGoldenTrophyIsolated(trophyX, trophyY, trophyZ);
    }
    
    glutSwapBuffers();
}


int main(int argc, char** argv) {
    //Inicialização do GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("RBD");

    initSaveSystem();
    init();
    
    //Renderização e Tempo
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    
    //Registro de Teclado
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    
    //Registro de Mouse
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);

    glutMainLoop();
    
    return 0;
}
