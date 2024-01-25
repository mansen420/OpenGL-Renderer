EXE := main.exe
SRC_DIR := src
IMGUI_DIR := imgui
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES += $(wildcard $(IMGUI_DIR)/*.cpp)
SRC_FILES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJ_FILES := $(addsuffix .o, $(basename $(notdir $(SRC_FILES))))
OBJ_FILES += glad.o
BIN_OBJ_FILES = $(addprefix $(BIN_DIR)/, $(OBJ_FILES))

CXXFLAGS := -Wall $(shell pkg-config --cflags glfw3) -Iinclude/ -I$(IMGUI_DIR)/ -I$(IMGUI_DIR)/backends
LIBS := $(shell pkg-config --static --libs glfw3)

$(BIN_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BIN_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BIN_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BIN_DIR)/$(EXE):$(BIN_OBJ_FILES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) && ./$@

$(BIN_DIR)/glad.o : $(SRC_DIR)/glad.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run : 
	./$(BIN_DIR)/$(EXE)