BIN_DIR		:= ./bin
SRC_DIR		:= ./src

BIN			:= $(BIN_DIR)/init-system
SRC			:= $(wildcard $(SRC_DIR)/*)

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p $(BIN_DIR)
	gcc $^ -o $@

clean:
	rm -rf $(BIN)