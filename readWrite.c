#include "readwrite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_FILE_NAME "save.txt"
#define MAX_LINE_LENGTH 256
#define MAX_CONFIGS 32
#define MAX_NAME_LENGTH 64
#define MAX_VALUE_LENGTH 128

static struct {
    char name[MAX_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
} configs[MAX_CONFIGS];

static int configCount = 0;
static int unlockedLevel = 1;

static void setConfigInternal(const char* name, const char* value) {
    for (int i = 0; i < configCount; i++) {
        if (strcmp(configs[i].name, name) == 0) {
            strncpy(configs[i].value, value, MAX_VALUE_LENGTH - 1);
            configs[i].value[MAX_VALUE_LENGTH - 1] = '\0';
            return;
        }
    }

    if (configCount < MAX_CONFIGS) {
        strncpy(configs[configCount].name, name, MAX_NAME_LENGTH - 1);
        configs[configCount].name[MAX_NAME_LENGTH - 1] = '\0';
        strncpy(configs[configCount].value, value, MAX_VALUE_LENGTH - 1);
        configs[configCount].value[MAX_VALUE_LENGTH - 1] = '\0';
        configCount++;
    }
}

static void parseSaveLine(const char* line) {
    if (line[0] == 'c' && line[1] == ':' ) {
        const char* text = line + 2;
        while (*text == ' ') ++text;

        const char* separator = strchr(text, ' ');
        if (!separator) return;

        char name[MAX_NAME_LENGTH];
        char value[MAX_VALUE_LENGTH];

        int nameLen = separator - text;
        if (nameLen <= 0 || nameLen >= MAX_NAME_LENGTH) return;
        strncpy(name, text, nameLen);
        name[nameLen] = '\0';

        const char* valueStart = separator + 1;
        while (*valueStart == ' ') ++valueStart;
        strncpy(value, valueStart, MAX_VALUE_LENGTH - 1);
        value[MAX_VALUE_LENGTH - 1] = '\0';

        char* newline = strchr(value, '\n');
        if (newline) *newline = '\0';
        char* carriage = strchr(value, '\r');
        if (carriage) *carriage = '\0';

        setConfigInternal(name, value);
    } else if (line[0] == 'l' && line[1] == ':' ) {
        const char* text = line + 2;
        while (*text == ' ') ++text;
        int level = atoi(text);
        if (level > unlockedLevel) {
            unlockedLevel = level;
        } else if (level > 0 && unlockedLevel <= 0) {
            unlockedLevel = level;
        }
    }
}

void initSaveSystem(void) {
    FILE* f = fopen(SAVE_FILE_NAME, "r");
    if (!f) {
        unlockedLevel = 1;
        configCount = 0;
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), f)) {
        parseSaveLine(line);
    }
    fclose(f);
}

void saveSaveFile(void) {
    FILE* f = fopen(SAVE_FILE_NAME, "w");
    if (!f) return;

    for (int i = 0; i < configCount; i++) {
        fprintf(f, "c:%s %s\n", configs[i].name, configs[i].value);
    }
    fprintf(f, "l:%d\n", unlockedLevel);
    fclose(f);
}

const char* getConfigValue(const char* name) {
    for (int i = 0; i < configCount; i++) {
        if (strcmp(configs[i].name, name) == 0) {
            return configs[i].value;
        }
    }
    return NULL;
}

void setConfigValue(const char* name, const char* value) {
    setConfigInternal(name, value);
    saveSaveFile();
}

int getUnlockedLevel(void) {
    return unlockedLevel;
}

void setUnlockedLevel(int level) {
    if (level > unlockedLevel) {
        unlockedLevel = level;
        saveSaveFile();
    }
}
