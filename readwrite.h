#ifndef READWRITE_H
#define READWRITE_H

#ifdef __cplusplus
extern "C" {
#endif

void initSaveSystem(void);
void saveSaveFile(void);
const char* getConfigValue(const char* name);
void setConfigValue(const char* name, const char* value);
int getUnlockedLevel(void);
void setUnlockedLevel(int level);

#ifdef __cplusplus
}
#endif

#endif // READWRITE_H
