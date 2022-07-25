TARGET_EXEC := nusstool
TEST_EXEC := test
CC=gcc

BIN_INSTALL_DIR := /usr/local/bin
LIB_INSTALL_DIR := /usr/local/lib 
INC_INSTALL_DIR := /usr/local/include/$(TARGET_EXEC)/
TYPE := bin # valid inputs: bin, a (static lib), so (shared lib) 

BUILD_DIR := ./build
BUILD_DIR_TEST := $(BUILD_DIR)/build_test
SRC_DIRS := ./src
INC_DIRS := ./include
EX_CC_FLAGS :=
EX_LD_FLAGS :=
# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s')
INCS := $(shell find $(SRC_DIRS) -name '*.h')

# String substitution for every C/C++ file.
# As an example, hello.c turns into ./build/hello.c.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./include will need to be passed to GCC so that it can find header files
# SRCS := $(shell find $(INC_DIRS) -name '*.h')

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Wpedantic -g $(EX_CC_FLAGS) -DTYPE=$(TYPE) -std=c99

LDFLAGS := $(EX_LD_FLAGS)

# The final build step.
# This builds a binary, shared or static library
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
ifeq ($(TYPE), a) 
	ar -rcs $@ $(OBJS)
else  ifeq ($(TYPE), so)
	$(CC) -shared $(OBJS) -o $@ $(LDFLAGS)
else 
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
endif 

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# Clean build 
.PHONY: build
build: 
	make clean
	make

# Build test task
.PHONY: build_test
build_test: 
	make EX_CC_FLAGS=-DTEST=1 EX_LD_FLAGS=-lcmocka TARGET_EXEC=$(TEST_EXEC) TYPE=bin 

.PHONY: test 
test: 
	make build_test BUILD_DIR=$(BUILD_DIR_TEST) 
	make run TARGET_EXEC=$(TEST_EXEC) BUILD_DIR=$(BUILD_DIR_TEST) TYPE=bin

# Run task 
.PHONY: run
run:
	make
	$(BUILD_DIR)/$(TARGET_EXEC)

.PHONY: leak
leak:
	valgrind $(BUILD_DIR)/$(TARGET_EXEC)


.PHONY: testleak
testleak:
	valgrind $(BUILD_DIR_TEST)/$(TEST_EXEC)

.PHONY: doc
doc: 
	clang-doc ./compile_commands.json -output=$(BUILD_DIR)/docs 

.PHONY: buildcmd 
buildcmd:
	make clean 
	bear -- make build

.PHONY: lint 
lint: 
	clang-tidy $(SRCS) $(INCS)

# Clean task
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# installs the binary, shared library or static library  
.PHONY: install 
install:
ifeq ($(BIN), a)
	mkdir -p $(INC_INSTALL_DIR) 
	cp -f $(BUILD_DIR)/$(TARGET_EXEC) $(LIB_INSTALL_DIR)
	for u in $(INC_DIRS); do echo $$u; cp -f $$u $(INC_INSTALL_DIR); done
else ifeq ($(BIN), so)
	mkdir -p $(INC_INSTALL_DIR) 
	cp -f $(BUILD_DIR)/$(TARGET_EXEC) $(LIB_INSTALL_DIR)
	for u in $(INC_DIRS); do echo $$u; cp -f $$u $(INC_INSTALL_DIR); done
else 
	mkdir -p $(BIN_INSTALL_DIR)
	cp -f $(BUILD_DIR)/$(TARGET_EXEC) $(BIN_INSTALL_DIR)
endif

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
