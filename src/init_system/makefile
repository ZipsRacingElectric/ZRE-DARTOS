BIN_DIR		:= ../../bin
SRC_DIR		:= .

C_BIN		:= $(BIN_DIR)/init-system
C_SRC		:= $(wildcard $(SRC_DIR)/*.c)
SH_SRC		:= $(wildcard $(SRC_DIR)/*.sh)
SH_BIN		:= $(patsubst $(SRC_DIR)/%, $(BIN_DIR)/%, $(SH_SRC))

all: $(C_BIN) $(SH_BIN)

$(BIN_DIR)/%.sh: $(SRC_DIR)/%.sh
	mkdir -p $(BIN_DIR)
	cp $< $@
	chmod a+x $@

$(C_BIN): $(C_SRC)
	mkdir -p $(BIN_DIR)
	gcc $^ -o $@ -lgpiod

clean:
	rm -rf $(BIN_DIR)