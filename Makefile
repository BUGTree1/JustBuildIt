rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
ifeq ($(OS),Windows_NT)
  EXE_EXT := .exe
  OBJ_EXT := .obj
else
  EXE_EXT :=
  OBJ_EXT := .o
endif

SRC_DIR := src
OUT_DIR := bin
OBJ_DIR := obj
OUT_FILE := buildit$(EXE_EXT)

DC := ldc2
DFLAGS := -O -I$(SRC_DIR) -I$(SRC_DIR)/libs

SRC_FILES := $(call rwildcard,$(SRC_DIR),*.d)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.d,$(OUT_DIR)/$(OBJ_DIR)/%$(OBJ_EXT),$(SRC_FILES))

.PHONY: test
test: $(OUT_DIR)/$(OUT_FILE)
	$(OUT_DIR)/$(OUT_FILE) -t cpp test

$(OUT_DIR)/$(OUT_FILE): $(OBJ_FILES)
	$(DC) $^ -of="$@"

$(OUT_DIR)/$(OBJ_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%.d
	$(DC) -c $< -of="$@" $(DFLAGS)