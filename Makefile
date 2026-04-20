CXX = g++
CXX_FLAGS = -static -Isrc -Wall -Wextra -O3 -std=c++17

OUT_DIR    = bin
CXX_SRC    = src/helper.cpp
CXX_OUT    = $(OUT_DIR)/buildit

LIB_SRC    = src/buildit.cpp
LIB_HEADER = src/buildit.h
LIB_OUT    = buildit.h

TEST_DIR   = test

.PHONY: run
run: build $(TEST_DIR)
	cd $(TEST_DIR) ; rm stdout.txt ; ../$(CXX_OUT) -t c test

.PHONY: build
build: $(LIB_OUT) $(CXX_OUT)

$(LIB_OUT): $(LIB_SRC) $(LIB_HEADER)
#	cat $(LIB_HEADER) > $(LIB_OUT)

$(CXX_OUT): $(OUT_DIR) $(LIB_SRC) $(LIB_HEADER) $(CXX_SRC)
	$(CXX) $(CXX_FLAGS) $(CXX_SRC) $(LIB_SRC) -o $(CXX_OUT)

$(OUT_DIR):
	mkdir $(OUT_DIR) || echo $(OUT_DIR) created
	
$(TEST_DIR):
	mkdir $(TEST_DIR) || echo $(TEST_DIR) created
