CC 		:= gcc

CFLAGS  := -Wall -Werror -pedantic -pthread -O2
LDFLAGS := -pthread -static

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := include

SRCS 	:= $(wildcard $(SRC_DIR)/*.c)
OBJS 	:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

TARGET 	:= $(BIN_DIR)/fabrial

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(LDFLAGS) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean
