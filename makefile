CXX = g++
EXE = cachesim
SRC-DIR = src
OBJ-DIR = obj
SRC-FILES = $(wildcard $(SRC-DIR)/*.cpp)
OBJ-FILES = $(patsubst $(SRC-DIR)/%.cpp, $(OBJ-DIR)/%.o, $(SRC-FILES))
STDFLAGS = --std=c++11
LFLAGS = -static-libstdc++

default: debug

clean:
	rm -r $(EXE) $(OBJ-DIR)

debug: OFLAGS = -Wall $(STDFLAGS)
debug: $(OBJ-DIR) $(OBJ-FILES)
	$(CXX) -o $(EXE) $(OBJ-FILES)

release: OFLAGS = -O3 $(STDFLAGS)
release: $(OBJ-DIR) $(OBJ-FILES) 
	$(CXX) $(LFLAGS) -o $(EXE) $(OBJ-FILES)

$(OBJ-DIR):
	mkdir -p $(OBJ-DIR)

$(OBJ-DIR)/%.o: $(SRC-DIR)/%.cpp
	$(CXX) $(OFLAGS) -c -o $@ $<
