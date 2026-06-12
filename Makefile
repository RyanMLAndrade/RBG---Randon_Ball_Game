TARGET = main1
CC = gcc
SRCS = main.c background.c gameEngine.c mapGenerator.c uiScreens.c playerCharacter.c
LDFLAGS = -lfreeglut -lopengl32 -lglu32
all: $(TARGET)
$(TARGET):
	$(CC) $(SRCS) -o $(TARGET) $(LDFLAGS)
	./$(TARGET).exe
clean:
	@if exist $(TARGET).exe del $(TARGET).exe
