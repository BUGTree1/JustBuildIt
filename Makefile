rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
ifeq ($(OS),Windows_NT)
  EXE_EXT := .exe
  OBJ_EXT := .obj
else
  EXE_EXT :=
  OBJ_EXT := .o
endif

SRC_EXT := .go

SRC_DIR := src
OUT_DIR := bin
OUT_FILE := buildit$(EXE_EXT)

COMPILER := go build
FLAGS := 

SRC_FILES := $(call rwildcard,$(SRC_DIR),*$(SRC_EXT))

.PHONY: test
test: $(OUT_DIR)/$(OUT_FILE)
	$(OUT_DIR)/$(OUT_FILE)
#	$(OUT_DIR)/$(OUT_FILE) -h
#	$(OUT_DIR)/$(OUT_FILE) -t cpp test

$(OUT_DIR)/$(OUT_FILE): $(SRC_FILES)
	$(COMPILER) -o $@ $(FLAGS) $^