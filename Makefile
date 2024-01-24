CC 				:= gcc
OBJ 			:= ./obj
INCLUDE 	:= ./include
SRC 			:= ./src
SRCS 			:= $(wildcard $(SRC)/*.c)
OBJS 			:= $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE 			:= simulate_bridge
DEBUG_EXE := debug_bridge
CFLAGS 		:= -Wall -g -I$(INCLUDE)
DFLAGS 		:= -DDEBUG

.PHONY: all run clean debug

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

debug: $(SRC)/main.c $(OBJ)/bridge.o
	$(CC) $(CFLAGS) $(DFLAGS) -o $(DEBUG_EXE) $^

run: $(EXE)
	./$(EXE) 0 3 15

test: debug
	./testing.sh

clean:
	rm -rf $(OBJ) $(EXE) $(DEBUG_EXE)
