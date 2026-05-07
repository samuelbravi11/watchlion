PROJECT     := watchlion
LIB_NAME    := lib$(PROJECT)
PREFIX     ?= /usr/local

CC          := gcc
CXX         := g++
AR          := ar
RM          := rm -rf
MKDIR_P     := mkdir -p

BUILD_DIR   := build
BIN_DIR     := bin
LIB_DIR     := lib
SRC_DIR     := src
INC_DIR     := include

CPPFLAGS    := -D_POSIX_C_SOURCE=200809L \
               -I$(INC_DIR) -I$(SRC_DIR) -I$(SRC_DIR)/events -I$(SRC_DIR)/parser -I$(SRC_DIR)/utils
CFLAGS      := -std=c11 -Wall -Wextra -Wpedantic -g -O2 -fPIC
CXXFLAGS    := -std=c++20 -Wall -Wextra -Wpedantic -g -O2 -fPIC
LDLIBS      := -pthread -lrt

LIB_C_SRCS  := $(SRC_DIR)/watchlion.c \
               $(SRC_DIR)/app.c \
               $(SRC_DIR)/events/counter.c \
               $(SRC_DIR)/events/queue_sig.c \
               $(SRC_DIR)/parser/parser.c \
               $(SRC_DIR)/utils/timer.c

LIB_CPP_SRCS := $(SRC_DIR)/utils/pid_hash_map.cpp

LIB_C_OBJS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIB_C_SRCS))
LIB_CPP_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LIB_CPP_SRCS))
LIB_OBJS     := $(LIB_C_OBJS) $(LIB_CPP_OBJS)

DAEMON_SRC   := app/main.c
DAEMON_OBJ   := $(BUILD_DIR)/$(DAEMON_SRC:.c=.o)

EXAMPLE_SRC  := example/example_main.c
EXAMPLE_OBJ  := $(BUILD_DIR)/$(EXAMPLE_SRC:.c=.o)

STATIC_LIB   := $(LIB_DIR)/$(LIB_NAME).a
SHARED_LIB   := $(LIB_DIR)/$(LIB_NAME).so
DAEMON_BIN   := $(BIN_DIR)/watchliond
EXAMPLE_BIN  := $(BIN_DIR)/example_main

.PHONY: all lib daemon example clean install uninstall dirs

all: lib daemon example

lib: $(STATIC_LIB) $(SHARED_LIB)

daemon: $(DAEMON_BIN)

example: $(EXAMPLE_BIN)

dirs:
	$(MKDIR_P) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR)

$(STATIC_LIB): $(LIB_OBJS) | dirs
	$(AR) rcs $@ $^

$(SHARED_LIB): $(LIB_OBJS) | dirs
	$(CXX) -shared -o $@ $^ $(LDLIBS)

$(DAEMON_BIN): $(DAEMON_OBJ) $(STATIC_LIB) | dirs
	$(CXX) -o $@ $(DAEMON_OBJ) $(STATIC_LIB) $(LDLIBS)

$(EXAMPLE_BIN): $(EXAMPLE_OBJ) $(STATIC_LIB) | dirs
	$(CXX) -o $@ $(EXAMPLE_OBJ) $(STATIC_LIB) $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

install: lib
	install -d $(DESTDIR)$(PREFIX)/include/watchlion
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(INC_DIR)/watchlion.h $(DESTDIR)$(PREFIX)/include/watchlion/watchlion.h
	install -m 644 $(STATIC_LIB) $(DESTDIR)$(PREFIX)/lib/$(LIB_NAME).a
	install -m 755 $(SHARED_LIB) $(DESTDIR)$(PREFIX)/lib/$(LIB_NAME).so

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/include/watchlion
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(LIB_NAME).a
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(LIB_NAME).so

clean:
	$(RM) $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR)