# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

TARGET := ami_tool
BUILD_DIR := ./build
SRC_DIRS := ./

# Find all C source files.
SRCS := $(shell find $(SRC_DIRS) -not -path "./test/*" -name '*.c')

# Prepends BUILD_DIR and appends .o to every src file.
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Replace .o with .d for dependencies.
DEPS := $(OBJS:.o=.d)

INC_DIRS := ../api/include ./cmd_handlers .
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

AMI_LIB_DIR := ../api/build
AMI_LIB := ami

CFLAGS := $(INC_FLAGS) -Wall -Werror
LDFLAGS := -L$(AMI_LIB_DIR) -l$(AMI_LIB) -lm -lpthread

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

#
# amiapp target
#
$(BUILD_DIR)/$(TARGET): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

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
