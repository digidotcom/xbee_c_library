# Makefile for the XBee Cellular Unix example

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../../include

# Directories
SRC_DIR = ../../../src
INC_DIR = ../../../include
BUILD_DIR = build
EXAMPLE_DIR = .
PORTS_DIR = ../../../ports

# Source files (core + cellular)
SRCS = \
	$(SRC_DIR)/xbee.c \
	$(SRC_DIR)/xbee_api_frames.c \
	$(SRC_DIR)/xbee_at_cmds.c \
	$(SRC_DIR)/xbee_cellular.c

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Example source files
EXAMPLE_SRCS = \
	$(EXAMPLE_DIR)/xbee_cellular_example.c \
	$(PORTS_DIR)/port_unix.c

EXAMPLE_OBJS = $(patsubst $(EXAMPLE_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter $(EXAMPLE_DIR)/%,$(EXAMPLE_SRCS))) \
               $(patsubst $(PORTS_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter $(PORTS_DIR)/%,$(EXAMPLE_SRCS)))

# Output binary
TARGET = xbee_cellular_example

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build rules
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(PORTS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(EXAMPLE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean target
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean