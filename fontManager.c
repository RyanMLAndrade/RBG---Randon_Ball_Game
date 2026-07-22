#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "stb_truetype.h"

// Estrutura global para nossa fonte customizada
GLuint fontTextureID;
stbtt_bakedchar bakedChars[96];
int fontAtlasSize = 512;

void initCustomFont(const char* fontPath, float fontSize) {
    // 1. Ler o arquivo .ttf em formato binário bruto
    FILE* f = fopen(fontPath, "rb");
    if (!f) {
        printf("Erro ao abrir a fonte: %s\n", fontPath);
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char* ttfBuffer = (unsigned char*)malloc(size);
    fread(ttfBuffer, 1, size, f);
    fclose(f);

    // Escolhe o tamanho do atlas com base no fontSize para preservar qualidade
    if (fontSize >= 48.0f) fontAtlasSize = 1024;
    else fontAtlasSize = 512;

    // 2. Criar um buffer temporário de pixels para renderizar o alfabeto
    unsigned char* bitmap = (unsigned char*)calloc(fontAtlasSize * fontAtlasSize, 1);

    // 3. A stb_truetype desenha todas as letras do arquivo .ttf dentro desse bitmap
    stbtt_BakeFontBitmap(ttfBuffer, 0, fontSize, bitmap, fontAtlasSize, fontAtlasSize, 32, 96, bakedChars);
    free(ttfBuffer);

    // 4. Transformar o bitmap gerado em uma textura oficial do OpenGL (Formato Alpha / Luminosidade)
    glGenTextures(1, &fontTextureID);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);

    // Garantir alinhamento correto de bytes (1 byte por pixel no bitmap)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Configura os filtros lineares fundamentais para o Anti-Aliasing funcionar perfeitamente
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, fontAtlasSize, fontAtlasSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    free(bitmap);
}

void drawHDText(float x, float y, const char* text) {
    if (fontTextureID == 0) return; // Fonte não inicializada
    // Salva estados GL que vamos alterar
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);
    GLfloat prevColor[4];
    glGetFloatv(GL_CURRENT_COLOR, prevColor);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING); // Garante que a cor venha pura do glColor


    // Desativa escrita no depth buffer e teste de profundidade para garantir overdraw
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float xpix = x;
    float ypix = y;
    while (*text) {
        if (*text >= 32 && *text < 128) {
            int ch = *text - 32;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(bakedChars, fontAtlasSize, fontAtlasSize, ch, &xpix, &ypix, &q, 1); // 1 = opengl fill

            glBegin(GL_QUADS);
                glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
                glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
                glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
                glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glEnd();
        }
        text++;
    }

    // Restaura estados de depth
    glDepthMask(GL_TRUE);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    // Restaura iluminação
    if (lightingEnabled) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);

    // Restaura blending
    if (!blendEnabled) glDisable(GL_BLEND);

    // Restaura textura
    if (!textureEnabled) glDisable(GL_TEXTURE_2D);
    // Restaura a cor corrente
    glColor4fv(prevColor);
}

int getTextWidthPixels(const char* text) {
    int width = 0;
    for (const char* c = text; *c != '\0'; c++) {
        if (*c >= 32 && *c < 128) {
            stbtt_bakedchar *b = bakedChars + (*c - 32);
            width += b->xadvance;
        }
    }
    return width;
}

void drawHDTextScaled(float x, float y, const char* text, float scale) {
    if (scale == 1.0f) {
        drawHDText(x, y, text);
        return;
    }

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    // draw at origin in scaled space
    drawHDText(0.0f, 0.0f, text);
    glPopMatrix();
}
