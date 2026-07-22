#include "drawObject.h"
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static OBJVertex* objVertices = NULL;
static OBJFace* objFaces = NULL;
static int numVertices = 0;
static int numFaces = 0;

// ID da Display List para renderização acelerada por hardware
static GLuint trophyDisplayList = 0;

// Função auxiliar interna para limpar formatos de face (ex: 1/1/1 ou 1//1 vira 1)
static int cleanIndex(const char* token) {
    int idx;
    sscanf(token, "%d", &idx);
    return idx;
}

// Função auxiliar interna para calcular a normal geométrica da face (Flat Shading)
static void calculateNormal(OBJVertex v1, OBJVertex v2, OBJVertex v3) {
    float ux = v2.x - v1.x;
    float uy = v2.y - v1.y;
    float uz = v2.z - v1.z;

    float vx = v3.x - v1.x;
    float vy = v3.y - v1.y;
    float vz = v3.z - v1.z;

    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;

    float length = (float)sqrt(nx * nx + ny * ny + nz * nz);
    if (length == 0.0f) length = 1.0f;

    glNormal3f(nx / length, ny / length, nz / length);
}

// Carrega o arquivo .obj usando strings e arquivos nativos do C
int loadObjectFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo OBJ: %s\n", filename);
        return 0;
    }

    // Inicializa contadores
    numVertices = 0;
    numFaces = 0;

    int maxVertices = 1000;
    int maxFaces = 1000;
    objVertices = (OBJVertex*)malloc(maxVertices * sizeof(OBJVertex));
    objFaces = (OBJFace*)malloc(maxFaces * sizeof(OBJFace));

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Ignora linhas vazias ou comentários
        if (line[0] == '\0' || line[0] == '\n' || line[0] == '#') continue;

        // Captura o identificador da linha (v, vn, f, etc)
        char type[10];
        if (sscanf(line, "%9s", type) != 1) continue;

        // Processa Vértices
        if (strcmp(type, "v") == 0) {
            if (numVertices >= maxVertices) {
                maxVertices *= 2;
                objVertices = (OBJVertex*)realloc(objVertices, maxVertices * sizeof(OBJVertex));
            }
            sscanf(line, "v %f %f %f", &objVertices[numVertices].x, &objVertices[numVertices].y, &objVertices[numVertices].z);
            numVertices++;
        }
        // Processa Faces de forma segura com strtok
        else if (strcmp(type, "f") == 0) {
            if (numFaces >= maxFaces) {
                maxFaces *= 2;
                objFaces = (OBJFace*)realloc(objFaces, maxFaces * sizeof(OBJFace));
            }

            // Pula o caractere 'f'
            char* token = strtok(line, " \t\r\n"); 
            char* vTokens[5];
            int count = 0;

            // Extrai os blocos da face
            while ((token = strtok(NULL, " \t\r\n")) != NULL && count < 4) {
                vTokens[count++] = token;
            }

            if (count >= 3) {
                objFaces[numFaces].v1 = cleanIndex(vTokens[0]) - 1;
                objFaces[numFaces].v2 = cleanIndex(vTokens[1]) - 1;
                objFaces[numFaces].v3 = cleanIndex(vTokens[2]) - 1;

                if (count == 4) {
                    objFaces[numFaces].v4 = cleanIndex(vTokens[3]) - 1;
                    objFaces[numFaces].isQuad = 1;
                } else {
                    objFaces[numFaces].v4 = -1;
                    objFaces[numFaces].isQuad = 0;
                }
                numFaces++;
            }
        }
    }
    fclose(file);

    // COMPILAÇÃO DA DISPLAY LIST (Otimização para projetos complexos)
    trophyDisplayList = glGenLists(1);
    glNewList(trophyDisplayList, GL_COMPILE);
    
    int i;
    for (i = 0; i < numFaces; i++) {
        if (objFaces[i].isQuad) {
            glBegin(GL_QUADS);
            calculateNormal(objVertices[objFaces[i].v1], objVertices[objFaces[i].v2], objVertices[objFaces[i].v3]);
            glVertex3f(objVertices[objFaces[i].v1].x, objVertices[objFaces[i].v1].y, objVertices[objFaces[i].v1].z);
            glVertex3f(objVertices[objFaces[i].v2].x, objVertices[objFaces[i].v2].y, objVertices[objFaces[i].v2].z);
            glVertex3f(objVertices[objFaces[i].v3].x, objVertices[objFaces[i].v3].y, objVertices[objFaces[i].v3].z);
            glVertex3f(objVertices[objFaces[i].v4].x, objVertices[objFaces[i].v4].y, objVertices[objFaces[i].v4].z);
            glEnd();
        } else {
            glBegin(GL_TRIANGLES);
            calculateNormal(objVertices[objFaces[i].v1], objVertices[objFaces[i].v2], objVertices[objFaces[i].v3]);
            glVertex3f(objVertices[objFaces[i].v1].x, objVertices[objFaces[i].v1].y, objVertices[objFaces[i].v1].z);
            glVertex3f(objVertices[objFaces[i].v2].x, objVertices[objFaces[i].v2].y, objVertices[objFaces[i].v2].z);
            glVertex3f(objVertices[objFaces[i].v3].x, objVertices[objFaces[i].v3].y, objVertices[objFaces[i].v3].z);
            glEnd();
        }
    }
    glEndList();

    // Como os dados já foram salvos direto na memória da GPU, liberamos a memória RAM local
    free(objVertices);
    free(objFaces);
    objVertices = NULL;
    objFaces = NULL;

    printf("Objeto compilado na GPU com sucesso. (%d faces)\n", numFaces);
    return 1;
}

// Método limpo de renderização solicitado
void drawObject(void) {
    if (trophyDisplayList != 0) {
        glCallList(trophyDisplayList);
    }
}

// Liberação de memória da GPU ao fechar o programa principal
void freeObjectMemory(void) {
    if (trophyDisplayList != 0) {
        glDeleteLists(trophyDisplayList, 1);
        trophyDisplayList = 0;
    }
}
