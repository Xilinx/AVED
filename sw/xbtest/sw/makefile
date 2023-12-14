# Declare inputs default values, see help for more info
ami=true
ami_cflags=default
ami_libs=default
xrt=false
xrt_cflags=default
xrt_libs=default
no_driver=false
internal_release=false
build_version=4064028
build_date=default
debug=false
code_coverage=false
checks=default
start_time := $(shell date -u +%s)

# AMI used by default unless xrt or no_driver option is provided
ifeq ($(xrt),true)
ami=false
else ifeq ($(no_driver),true)
ami=false
endif

SRC_PATH = ./src
INCLUDE_PATH = ./include
OUTPUT_DIR = ./output

ifeq ($(ami),true)
#ami compile and link options
LIBINFO_AMI = "AMI info not available"

ifeq ($(ami_cflags),default)
CFLAGS_AMI  = $(shell pkg-config --cflags         ami)
LIBINFO_AMI = $(shell pkg-config --print-provides ami)
else
CFLAGS_AMI  = $(ami_cflags)
endif

ifeq ($(ami_libs),default)
LIBS_AMI    = $(shell pkg-config --libs         ami)
LIBINFO_AMI = $(shell pkg-config --print-provides ami)
else
LIBS_AMI    = $(ami_libs)
endif

else ifeq ($(xrt),true)

#xrt compile and link options
LIBINFO_XRT = "XRT info not available"

ifeq ($(xrt_cflags),default)
CFLAGS_XRT  = $(shell pkg-config --cflags         xrt)
LIBINFO_XRT = $(shell pkg-config --print-provides xrt)
else
CFLAGS_XRT  = $(xrt_cflags)
endif

ifeq ($(xrt_libs),default)
# xrt for openCL support. Note: should -lxilinxopencl be present in the output of pkg-config --libs xrt
LIBS_XRT    = $(shell pkg-config --libs           xrt)  -luuid -lxrt_coreutil -lxilinxopencl
LIBINFO_XRT = $(shell pkg-config --print-provides xrt)
else
LIBS_XRT    = $(xrt_libs)
endif
endif

# json-glib compile and link options
CFLAGS_JSON_GLIB    = $(shell pkg-config --cflags         json-glib-1.0)
LIBS_JSON_GLIB      = $(shell pkg-config --libs           json-glib-1.0)
LIBINFO_JSON_GLIB   = $(shell pkg-config --print-provides json-glib-1.0)

# ncurses compile and link options
CFLAGS_NCURSES  = $(shell pkg-config --cflags         ncurses)
LIBS_NCURSES    = $(shell pkg-config --libs           ncurses)
LIBINFO_NCURSES = $(shell pkg-config --print-provides ncurses)

# Enable warnings
WFLAGS  = -Wall
WFLAGS += -Wextra
# WFLAGS += -Wdouble-promotion
# WFLAGS += -Wunused
# WFLAGS += -Wclobbered
# WFLAGS += -Wempty-body
# WFLAGS += -Wignored-qualifiers
# WFLAGS += -Wmissing-field-initializers
# WFLAGS += -Wtype-limits
# WFLAGS += -Wuninitialized
# WFLAGS += -Wunused-parameter
# WFLAGS += -Wunused-but-set-parameter

# Define the executable path
EXE_XBTEST_DEBUG = $(OUTPUT_DIR)/xbtest_debug
EXE_XBTEST = $(OUTPUT_DIR)/xbtest
LIBXBTEST_A = $(OUTPUT_DIR)/libxbtest.a

# Host application header files directory
INCLUDE_DIRS  = -I$(INCLUDE_PATH)/xbtest_sw
INCLUDE_DIRS += -I$(INCLUDE_PATH)/xbtest_common
INCLUDE_DIRS += -I$(INCLUDE_PATH)/share
INCLUDE_DIRS += -I$(INCLUDE_PATH)/api

# Host application source files directory
SRC_DIRS  = $(SRC_PATH)/xbtest_sw
SRC_DIRS += $(SRC_PATH)/xbtest_common
SRC_DIRS += $(SRC_PATH)/share
SRC_DIRS += $(SRC_PATH)/api

# Path to all source files
SRC_CPPS = $(shell find $(SRC_DIRS) -maxdepth 1 -name \*.cpp)

# Extract the CPP file name without extension
SRC_CPPS_NAMES = $(basename $(notdir $(SRC_CPPS)))

# Define object path for each CPP: Prepends OUTPUT_DIR and appends .o to every src file.
OBJS := $(SRC_CPPS_NAMES:%=$(OUTPUT_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Define objects to be included in libxbtest.a
A_OBJS := $(patsubst %main.o,,$(OBJS))

# Define objects to be used to build executables (xbtest and xbtest_debug)
EXE_OBJS := $(filter %main.o,$(OBJS))
XBTEST_EXE_OBJS := $(filter %xbtestcommonmain.o,$(OBJS))
XBTEST_DEBUG_EXE_OBJS := $(filter %xbtestswmain.o,$(OBJS))

# Define Preprocess, compile and assemble options
CPPFLAGS	 = -std=c++14 -MMD -MP -Os $(WFLAGS) $(CFLAGS_JSON_GLIB) $(CFLAGS_NCURSES) $(INCLUDE_DIRS)
EXE_CPPFLAGS = -std=c++14 -MMD -MP -Os $(WFLAGS) $(CFLAGS_JSON_GLIB) $(CFLAGS_NCURSES) -I$(INCLUDE_PATH)/api

# Define link options
LDFLAGS = $(LIBS_JSON_GLIB) $(LIBS_NCURSES) -lpthread -lrt -lstdc++

# Library information
LIBINFO = $(LIBINFO_JSON_GLIB), $(LIBINFO_NCURSES)

ifeq ($(no_driver),true)
	CPPFLAGS += -DUSE_NO_DRIVER
else ifeq ($(ami),true)
	CPPFLAGS += $(CFLAGS_AMI) -DUSE_AMI
	EXE_CPPFLAGS += $(CFLAGS_AMI)
	LDFLAGS  += $(LIBS_AMI)
	LIBINFO  += , $(LIBINFO_AMI)
else ifeq ($(xrt),true)
	CPPFLAGS += $(CFLAGS_XRT) -DUSE_XRT
	EXE_CPPFLAGS += $(CFLAGS_XRT)
	LDFLAGS  += $(LIBS_XRT)
	LIBINFO  += , $(LIBINFO_XRT)
endif

EXE_LDFLAGS = -L$(OUTPUT_DIR) -lxbtest $(LDFLAGS)

# Archive generation options
AR := ar
AR_FLAGS := -rc

# Get compiler information
CXX_VERSION = $(shell $(CXX) -dumpversion)
CXX_MACHINE = $(shell $(CXX) -dumpmachine)

# Get host information
OS_ID = $(shell lsb_release -is)
OS_VERSION = $(shell lsb_release -rs)
OS_VERSION_LOWER_THAN_8 = $(shell expr `echo $(OS_VERSION) | cut -f1 -d.` \< 8)

## if (OS = (centos or redhat)) and (OS version <= 7) ) => enable devtool
ifeq ($(OS_ID),RedHatEnterpriseServer)
	REDHAT = 1
else ifeq ($(OS_ID),CentOS)
	REDHAT = 1
else
	REDHAT = 0
endif

ifeq ($(REDHAT),1)
ifeq ($(OS_VERSION_LOWER_THAN_8),1)
	DEVTOOL = 1
else
	DEVTOOL = 0
endif
else
	DEVTOOL = 0
endif

# Code coverage
ifeq ($(code_coverage), true)
	CPPFLAGS += --coverage
	EXE_CPPFLAGS += --coverage
	LDFLAGS  += --coverage
	EXE_LDFLAGS  += --coverage
endif

# Debug flags
# in debug, disable optimization (overwrite -Os) and turn on debug info production with -g
ifeq ($(debug), true)
	CPPFLAGS += -O0 -g
	EXE_CPPFLAGS += -O0 -g
	LDFLAGS  += -g
	EXE_LDFLAGS  += -g
endif

# Build date and version macros
CPPFLAGS += -DBUILD_VERSION=$(build_version)
ifeq ($(build_date), default)
	CPPFLAGS += -DBUILD_DATE="$(shell date)"
else
	CPPFLAGS += -DBUILD_DATE="$(build_date)"
endif

# Internal flag macro
ifeq ($(internal_release), true)
	CPPFLAGS += -DINTERNAL_RELEASE
endif

# clang-tidy checks definition
CT_CHECKS = "-*,modernize-*,performance-*,portability-*,readability-*,-readability-implicit-bool-cast,cert-*,clang-analyzer-*"
ifneq ($(checks), default)
	CT_CHECKS = $(checks)
endif

# Makefile rules

.PHONY: all
all:
	@$(MAKE) --no-print-directory info
	@$(MAKE) --no-print-directory $(EXE_XBTEST_DEBUG) $(EXE_XBTEST)
	@$(MAKE) --no-print-directory end start_time=$(start_time)

info:
	@echo ""
	@echo "INFO: [XBTEST_MAKEFILE-2] Inputs:"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - no_driver        : $(no_driver)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - ami              : $(ami)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - ami_cflags       : $(ami_cflags)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - ami_libs         : $(ami_libs)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - xrt              : $(xrt)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - xrt_cflags       : $(xrt_cflags)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - xrt_libs         : $(xrt_libs)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - internal_release : $(internal_release)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - build_version    : $(build_version)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - build_date       : $(build_date)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - debug            : $(debug)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - code_coverage    : $(code_coverage)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - checks           : $(checks)"
	@echo ""
	@echo "INFO: [XBTEST_MAKEFILE-2] Environment:"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - USER             : $(USER)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - PWD              : $(PWD)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - OS               : $(OS_ID) $(OS_VERSION)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - CXX              : $(CXX) $(CXX_VERSION) $(CXX_MACHINE)"
	@echo "INFO: [XBTEST_MAKEFILE-2]   - LIB              : $(LIBINFO)"

end:
	@end_time=`date -u +%s`; elapsed=`expr $$end_time - $(start_time)`; echo "INFO: [XBTEST_MAKEFILE-4] Total elapsed time: $$elapsed seconds"
	@echo "INFO: [XBTEST_MAKEFILE-3] ...Done"

$(A_OBJS):
	@mkdir -p $(OUTPUT_DIR)
ifeq "$(DEVTOOL)" "1"
	scl enable devtoolset-9 '$(CXX) $(CPPFLAGS) -c $(shell find $(SRC_DIRS) -maxdepth 1 -name $(basename $(notdir $(@))).cpp) -o $(@)'
else
	$(CXX) $(CPPFLAGS) -c $(shell find $(SRC_DIRS) -maxdepth 1 -name $(basename $(notdir $(@))).cpp) -o $(@)
endif

$(LIBXBTEST_A): $(A_OBJS)
	$(AR) $(AR_FLAGS) $(LIBXBTEST_A) $(A_OBJS)
	@echo "INFO: [XBTEST_MAKEFILE-4] Library archive generated: $(LIBXBTEST_A)"

$(EXE_OBJS):
	@mkdir -p $(OUTPUT_DIR)
ifeq "$(DEVTOOL)" "1"
	scl enable devtoolset-9 '$(CXX) $(EXE_CPPFLAGS) -c $(shell find $(SRC_DIRS) -maxdepth 1 -name $(basename $(notdir $(@))).cpp) -o $(@)'
else
	$(CXX) $(EXE_CPPFLAGS) -c $(shell find $(SRC_DIRS) -maxdepth 1 -name $(basename $(notdir $(@))).cpp) -o $(@)
endif

$(EXE_XBTEST_DEBUG): $(LIBXBTEST_A) $(XBTEST_DEBUG_EXE_OBJS)
ifeq "$(DEVTOOL)" "1"
	scl enable devtoolset-9 '$(CXX) $(XBTEST_DEBUG_EXE_OBJS) $(EXE_LDFLAGS) -o $(EXE_XBTEST_DEBUG)'
else
	$(CXX) $(XBTEST_DEBUG_EXE_OBJS) $(EXE_LDFLAGS) -o $(EXE_XBTEST_DEBUG)
endif
	@echo "INFO: [XBTEST_MAKEFILE-5] Host application generated: $(EXE_XBTEST_DEBUG)"

$(EXE_XBTEST): $(LIBXBTEST_A) $(XBTEST_EXE_OBJS)
ifeq "$(DEVTOOL)" "1"
	scl enable devtoolset-9 '$(CXX) $(XBTEST_EXE_OBJS) $(EXE_LDFLAGS) -o $(EXE_XBTEST)'
else
	$(CXX) $(XBTEST_EXE_OBJS) $(EXE_LDFLAGS) -o $(EXE_XBTEST)
endif
	@echo "INFO: [XBTEST_MAKEFILE-5] Host application generated: $(EXE_XBTEST)"

.PHONY: clean
clean:
	@echo "INFO: [XBTEST_MAKEFILE-6] Removing output directory $(OUTPUT_DIR)"
	@$(RM) -rf $(OUTPUT_DIR)

check: $(SRC_CPPS)
	@echo "INFO: [XBTEST_MAKEFILE-7] Removing clang_tidy.log"
	@$(RM) -f clang_tidy.log
	@echo "INFO: [XBTEST_MAKEFILE-7] Running clang-tidy. Output captured in clang_tidy.log"
	scl enable llvm-toolset-7 'clang-tidy -checks=$(CT_CHECKS) -header-filter=.* --warnings-as-errors=* $(SRC_CPPS) -- $(CPPFLAGS) | tee -a clang_tidy.log'

space :=
space +=
join_slash := " / "

help:
	@echo ""
	@echo "INFO: [XBTEST_MAKEFILE-1] The following rules are available:"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - $(LIBXBTEST_A) : Generate libxbtest.a archive"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - $(EXE_XBTEST) : Only link the compiled object and generate the xbtest executable"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - $(EXE_XBTEST_DEBUG) : Only link the compiled object and generate the xbtest debug executable"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - all : Generate all output above"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - clean : Remove output directory"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - check : Run clang-tidy tool"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - info : Print makefile info"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - help : Print this message"
	@echo ""
	@echo "INFO: [XBTEST_MAKEFILE-1] The following inputs are available:"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - ami              : Build for AMI (default)"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - ami_cflags       : Compile flags for AMI"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - ami_libs         : Link flags for AMI"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - xrt              : Build for XRT instead of AMI"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - xrt_cflags       : Compile flags for XRT"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - xrt_libs         : Link flags for XRT"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - no_driver        : DEBUG - Use low level peek-poke instead of AMI (default) and do not get sensors (only supported for xbtest debug executable)"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - internal_release : Attach 'INTERNAL' flags in the host application"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - build_version    : Build version used as #define preprocessor directive"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - build_date       : Build date used as #define preprocessor directive"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - debug            : Build host application with debug info to run application with debbuger"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - code_coverage    : Enable --coverage $(CXX) option"
	@echo "INFO: [XBTEST_MAKEFILE-1]	   - checks           : Use 'make check' to run clang-tidy. Define clang-tidy option providing 'checks=<opt>' or use default"

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

# Copyright (C) 2022 Xilinx, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
