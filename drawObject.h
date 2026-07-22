#ifndef DRAW_OBJECT_H
#define DRAW_OBJECT_H

// Estruturas de dados básicas para a geometria
typedef struct {
    float x, y, z;
} OBJVertex;

typedef struct {
    int v1, v2, v3, v4;
    int isQuad;
} OBJFace;

// Protótipos das funções que serão chamadas no seu projeto principal
int loadObjectFile(const char* filename);
void drawObject(void);
void freeObjectMemory(void);

#endif // DRAW_OBJECT_H