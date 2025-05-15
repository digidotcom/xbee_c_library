# Makefile for the XBee LR library Unix example

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR)

# Directories
SRC_DIR     = ../../../src
INC_DIR     = ../../../include
PORTS_DIR   = ../../../ports
BUILD_DIR   = build
EXAMPLE_DIR = .

# Source files (core logic + LR implementation)
SRCS = \
	$(SRC_DIR)/xbee.c \
	$(SRC_DIR)/xbee_api_frames.c \
	$(SRC_DIR)/xbee_at_cmds.c \
	$(SRC_DIR)/xbee_lr.c

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Example and platform-specific source files
EXAMPLE_SRCS = \
	$(EXAMPLE_DIR)/xbee_lr_example.c \
	$(PORTS_DIR)/port_unix.c

EXAMPLE_OBJS = \
	$(patsubst $(EXAMPLE_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter $(EXAMPLE_DIR)/%,$(EXAMPLE_SRCS))) \
	$(patsubst $(PORTS_DIR)/%.c,$(BUILD_DIR)/%.o,$(filter $(PORTS_DIR)/%,$(EXAMPLE_SRCS)))

# Output binary
TARGET = xbee_lr_example

# Default rule
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compilation rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLE_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(PORTS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linking rule
$(TARGET): $(OBJS) $(EXAMPLE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean