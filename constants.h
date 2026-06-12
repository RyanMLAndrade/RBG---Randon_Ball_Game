#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdbool.h>

#define SKY_CYAN 0.0, 0.8, 1.0          // Ciano do topo
#define SKY_LIGHT_BLUE 0.5, 0.9, 1.0    // Azul claro do meio
#define OCEAN_TOP 0.0, 0.3, 0.6         // Mar horizonte
#define OCEAN_BOTTOM 0.0, 0.1, 0.4      // Mar fundo 

extern float cameraX, cameraZ, speed, lateralSpeed;
extern float maxSpeed, minSpeed, accelVertical, dragVertical;
extern float accelLateral, maxLateralSpeed, blockWidth;

extern float cameraY, jumpSpeed, gravity, groundY;
extern float deathBallYOffset;
extern float jumpAscendForce, maxJumpSpeed;
extern bool isJumping;
extern bool isDead;

extern int jumpTimer;          // Contador de frames do pulo atual
extern int minJumpFrames;      // Duração mínima obrigatória
extern int maxJumpFrames;      // Duração máxima permitida

extern bool keyStates[256];        // Teclas normais
extern bool specialKeyStates[256]; // Teclas especiais

#endif
