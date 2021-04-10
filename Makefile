CC = g++
CFLAGS = -g
SRC_DIR = src
INC_DIR = .

# These files compile via implicit rules
veco: ${SRC_DIR}/veco.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(INC_DIR) $< -o $@

clean:
	rm -f veco
