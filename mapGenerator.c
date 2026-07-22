#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mapGenerator.h"
#include "constants.h"

typedef struct {
    int levelNumber;
    int seed;
    float freqMin;
    float freqMax;
    float thresholdMin;
    float thresholdMax;
    float difficultyRate;
    int levelEnd;
    float lightDirection[4];
    float lightDiffuse[4];
    float lightSpecular[4];
    float lightAmbient[4];
} LevelConfig;

static const LevelConfig levelPresets[] = {
    {
        .levelNumber = 1,
        .seed = 1011,
        .freqMin = 0.1f,
        .freqMax = 0.1f,
        .thresholdMin = 0.40f,      //0 = tudo é bloco, 1 = tudo é buraco
        .thresholdMax = 0.50f,
        .difficultyRate = 1500.0f,  // Quanto menor mais rápido a dificuldade aumenta
        .levelEnd = 500,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, // muda cor do nível
        .lightSpecular = { 1.0f, 0.9f, 1.0f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    },
    {
        .levelNumber = 2,
        .seed = 1002,
        .freqMin = 0.10f,
        .freqMax = 0.22f,
        .thresholdMin = 0.35f,
        .thresholdMax = 0.50f,
        .difficultyRate = 700.0f,
        .levelEnd = 600,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.0f, 0.9f, 1.2f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    },
    {
        .levelNumber = 3,
        .seed = 1001,
        .freqMin = 0.14f,
        .freqMax = 0.26f,
        .thresholdMin = 0.35f,
        .thresholdMax = 0.52f,
        .difficultyRate = 1200.0f,
        .levelEnd = 700,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.0f, 0.9f, 1.4f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    },
    {
        .levelNumber = 4,
        .seed = 1004,
        .freqMin = 0.16f,
        .freqMax = 0.27f,
        .thresholdMin = 0.35f,
        .thresholdMax = 0.50f,
        .difficultyRate = 1200.0f,
        .levelEnd = 700,
        .lightDirection = { 0.0f, 0.8f, 0.8f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.0f, 0.9f, 1.6f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    }
    ,
    {
        .levelNumber = 5,
        .seed = 1005,
        .freqMin = 0.17f,
        .freqMax = 0.26f,
        .thresholdMin = 0.30f,
        .thresholdMax = 0.55f,
        .difficultyRate = 1200.0f,
        .levelEnd = 800,
        .lightDirection = { 0.0f, 0.7f, 0.75f, 0.0f }, // luz mais amena
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.0f, 0.9f, 1.2f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    }
    ,
    {
        .levelNumber = 6,
        .seed = 1006,
        .freqMin = 0.24f,
        .freqMax = 0.26f,
        .thresholdMin = 0.35f,
        .thresholdMax = 0.55f,
        .difficultyRate = 1200.0f,
        .levelEnd = 850,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.3f, 0.9f, 1.0f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    },
    {
        .levelNumber = 7,
        .seed = 1007,
        .freqMin = 0.24f,
        .freqMax = 0.28f,
        .thresholdMin = 0.35f,
        .thresholdMax = 0.60f,
        .difficultyRate = 1200.0f,
        .levelEnd = 950,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.4f, 0.9f, 1.0f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    }
    ,
    {
        .levelNumber = 8,
        .seed = 1008,
        .freqMin = 0.24f,
        .freqMax = 0.29f,
        .thresholdMin = 0.39f,
        .thresholdMax = 0.62f,
        .difficultyRate = 1200.0f,
        .levelEnd = 1000,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.5f, 0.9f, 1.0f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    },
    {
        .levelNumber = 9,
        .seed = 1009,
        .freqMin = 0.14f,
        .freqMax = 0.35f,
        .thresholdMin = 0.41f,
        .thresholdMax = 0.65f,
        .difficultyRate = 1200.0f,
        .levelEnd = 1050,
        .lightDirection = { 0.0f, 1.0f, 0.9f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.8f, 1.0f }, 
        .lightSpecular = { 1.0f, 0.9f, 3.0f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.15f, 1.0f }
    }
    ,
    {
        .levelNumber = 10,
        .seed = 1010,
        .freqMin = 0.24f,
        .freqMax = 0.50f,
        .thresholdMin = 0.50f,
        .thresholdMax = 0.70f,
        .difficultyRate = 1200.0f,
        .levelEnd = 1200,
        .lightDirection = { 0.2f, 0.85f, 0.75f, 0.0f },
        .lightDiffuse = { 1.0f, 0.9f, 0.6f, 1.0f },
        .lightSpecular = { 1.0f, 0.9f, 3.7f, 1.0f },
        .lightAmbient = { 0.2f, 0.15f, 0.2f, 1.0f }
    }
    
};

static const int levelPresetCount = sizeof(levelPresets) / sizeof(levelPresets[0]);
static int selectedLevelIndex = 0;
static int levelEnd = 0;
static int mapSeed = 0;
static float mapFreqMin = 0.08f;
static float mapFreqMax = 0.22f;
static float mapThresholdMin = 0.28f;
static float mapThresholdMax = 0.42f;
static float difficultyRate = 1500.0f;
static float levelLightDirection[4] = { 0.0f, 1.0f, 0.9f, 0.0f };
static float levelLightDiffuse[4] = { 1.0f, 1.0f, 0.8f, 1.0f };
static float levelLightSpecular[4] = { 1.0f, 1.0f, 0.8f, 1.0f };
static float levelLightAmbient[4] = { 0.2f, 0.17f, 0.16f, 1.0f };
static int levelConfigInitialized = 0;

static void applyLevelPreset(int presetIndex) {
    if (presetIndex < 0 || presetIndex >= levelPresetCount) return;
    const LevelConfig* preset = &levelPresets[presetIndex];
    selectedLevelIndex = presetIndex;
    mapSeed = preset->seed;
    mapFreqMin = preset->freqMin;
    mapFreqMax = preset->freqMax;
    mapThresholdMin = preset->thresholdMin;
    mapThresholdMax = preset->thresholdMax;
    difficultyRate = preset->difficultyRate;
    levelEnd = preset->levelEnd;
    memcpy(levelLightDirection, preset->lightDirection, sizeof(levelLightDirection));
    memcpy(levelLightDiffuse, preset->lightDiffuse, sizeof(levelLightDiffuse));
    memcpy(levelLightSpecular, preset->lightSpecular, sizeof(levelLightSpecular));
    memcpy(levelLightAmbient, preset->lightAmbient, sizeof(levelLightAmbient));
    levelConfigInitialized = 1;
}

static void ensureLevelConfigInitialized(void) {
    if (!levelConfigInitialized) {
        applyLevelPreset(0);
    }
}

float getNoise(int x, int seed) {
    int n = x + seed * 57;
    n = (n << 13) ^ n;
    int res = (n * (n * n * 15131 + 789224) + 137631258) & 0x7fffffff;
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
    ensureLevelConfigInitialized();

    if (index < 0) return 1;
    if (index < 7) return 1;

    int seed = mapSeed;
    float difficultyFactor = (index / difficultyRate);
    if (difficultyFactor > 1.0f) difficultyFactor = 1.0f;

    float frequency = mapFreqMin + (mapFreqMax - mapFreqMin) * difficultyFactor;
    float threshold = mapThresholdMin + (mapThresholdMax - mapThresholdMin) * difficultyFactor;

    float noiseValue = perlin1D(index * frequency, seed + (col * 1000));
    return (noiseValue > threshold) ? 1 : 0;
} 

void drawCube(float x, float z, int i, int col) {
    float yTop = 0.0f;
    float yBottom = -0.6f;
    float xRight = x + blockWidth;
    
    float zFront = z;
    float zBack = z - 1.0f;

    glBegin(GL_QUADS);
        // face de cima 
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
    ensureLevelConfigInitialized();
    int currentIdx = (int)(-cameraZ);
    
    int startIdx = currentIdx - 7;
    //if (startIdx < 0) startIdx = 0;

    for (int i = startIdx; i < currentIdx + 90; i++) {
        for (int col = 0; col < 5; col++) {
            if (getMapValue(i, col) == 1) {
                drawCube(col * blockWidth, -i, i, col);
            }
        }
    }
}

int getSelectedLevel(void) {
    ensureLevelConfigInitialized();
    return levelPresets[selectedLevelIndex].levelNumber;
}

void setSelectedLevel(int level) {
    for (int i = 0; i < levelPresetCount; i++) {
        if (levelPresets[i].levelNumber == level) {
            applyLevelPreset(i);
            return;
        }
    }
    if (level < levelPresets[0].levelNumber) {
        applyLevelPreset(0);
    } else {
        applyLevelPreset(levelPresetCount - 1);
    }
}

int getLevelEnd(void) {
    ensureLevelConfigInitialized();
    return levelEnd;
}

int getMaxLevelPreset(void) {
    return levelPresetCount;
}

void getCurrentLevelLighting(float* direction, float* diffuse, float* specular, float* ambient) {
    ensureLevelConfigInitialized();
    memcpy(direction, levelLightDirection, sizeof(levelLightDirection));
    memcpy(diffuse, levelLightDiffuse, sizeof(levelLightDiffuse));
    memcpy(specular, levelLightSpecular, sizeof(levelLightSpecular));
    memcpy(ambient, levelLightAmbient, sizeof(levelLightAmbient));
}
