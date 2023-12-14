# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

# Makefile based on https://makefiletutorial.com/

AR := ar -rc

TARGET := libami.a
BUILD_DIR := ./build
SRC_DIRS := ./src  # Private source/headers

# Find all C source files.
SRCS := $(shell find $(SRC_DIRS) -name '*.c')

# Prepends BUILD_DIR and appends .o to every src file.
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Replace .o with .d for dependencies.
DEPS := $(OBJS:.o=.d)

INC_DIRS := ./include  # Public headers only
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us.
# These files will have .d instead of .o as the output.
CFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Werror

#
# Default make
#
all:
	@$(MAKE) -s banner
	@$(MAKE) -s $(BUILD_DIR)/$(TARGET)

#
# Prints new line.
#
newline:
	@echo "#######################################################################\n"

#
# Displays important info at the start of the make.
#
banner:
	@$(MAKE) -s newline
	@echo "  - $(shell date)"
	@echo "  - $(USER)"
	@echo "  - $(PWD)\n"
	@echo "  - CC          : $(CC)"
	@echo "  - CFLAGS      : $(CFLAGS)"
	@echo "  - INC_DIRS    : $(INC_DIRS)"
	@echo "  - BUILD_DIR   : $(BUILD_DIR)\n"
	@$(MAKE) -s newline

# The final build step.
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@echo "Building $@"
	$(AR) $@ $^
	@echo "Done\n"
	@$(MAKE) -s newline

# Build step for C source.
$(BUILD_DIR)/%.c.o: %.c
	@echo "Building $@"
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
