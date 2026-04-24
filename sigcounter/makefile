CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L -g -Iinclude
LDFLAGS =
TARGET  = sigcounter

SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

SRCS = sigcounter.c \
       $(SRC_DIR)/app.c \
       $(SRC_DIR)/counter.c \
       $(SRC_DIR)/queue_sig.c

OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) 5

clean:
	rm -rf $(OBJ_DIR) $(TARGET)