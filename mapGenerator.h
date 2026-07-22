#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

void renderMap();
int getMapValue(int index, int col);
void drawSquare(float x, float z);

int getSelectedLevel(void);
void setSelectedLevel(int level);
int getLevelEnd(void);
int getMaxLevelPreset(void);
void getCurrentLevelLighting(float* direction, float* diffuse, float* specular, float* ambient);

#endif
