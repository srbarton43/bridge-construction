CC 			:= gcc
OBJ 		:= ./obj
INCLUDE := ./include
SRC 		:= ./src
SRCS 		:= $(wildcard $(SRC)/*.c)
OBJS 		:= $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE 		:= simulate_bridge
CFLAGS 	:= -Wall -g -I$(INCLUDE)

.PHONY: all run clean

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $(OBJS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir $@

#headers
$(OBJ)/main.o: $(INCLUDE)/bridge.h
$(OBJ)/vehicle.o: $(INCLUDE)/bridge.h

run: $(EXE)
	./$(EXE)

clean:
	rm -rf $(OBJ) $(EXE)
