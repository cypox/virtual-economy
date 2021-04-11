CC = g++
CFLAGS = -std=c++2a
DEBUG = -g -O0
SRC_DIR = src
INC_DIR = .
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# These files compile via implicit rules
veco: ${SRC_DIR}/veco.cpp
	$(CC) $(CFLAGS) $(DEBUG) $(CPPFLAGS) -I$(INC_DIR) $< -o $@ $(SFML_LIBS)

.PHONY: veco clean

clean:
	rm -f veco
