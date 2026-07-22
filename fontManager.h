#ifndef FONTMANAGER_H
#define FONTMANAGER_H

void initCustomFont(const char* fontPath, float fontSize);
void drawHDText(float x, float y, const char* text);
void drawHDTextScaled(float x, float y, const char* text, float scale);
int getTextWidthPixels(const char* text);

#endif