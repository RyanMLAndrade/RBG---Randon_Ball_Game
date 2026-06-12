#include <GL/glut.h>
#include "gameEngine.h"
#include "background.h"
#include "mapGenerator.h"
#include "playerCharacter.h"
#include "uiScreens.h"
#include "constants.h"

// --- ERRO CORRIGIDO 1: Informa ao main que o ID da textura existe no gameEngine.c ---
extern GLuint texturaSombraID; 
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 1. Configura a Câmera (Padrão funcional do seu jogo)
    gluLookAt(cameraX, cameraY, 8.0 + cameraZ, 
              cameraX, cameraY - 0.8f, cameraZ - 2.0, 
              0.0, 1.0, 0.0);

    // 2. Desenha o Cenário de Fundo
    glDisable(GL_DEPTH_TEST);
        drawBackground();
        drawMountain();  
        drawClouds();
    glEnable(GL_DEPTH_TEST);
    
    // 3. Desenha a Pista primeiro
    renderMap();

    // 4. Desenha o Personagem (Ele mesmo vai gerenciar a própria sombra agora)
    drawPlayer();

    // 5. Efeitos e HUD
    drawFog3D();
    drawHUD();

    glutSwapBuffers();
}

// O restante das suas funções (main, reshape, etc.) continuam iguais aqui para baixo...

int main(int argc, char** argv) {
    //Inicialização do GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960, 540);
    glutCreateWindow("Teste");

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
