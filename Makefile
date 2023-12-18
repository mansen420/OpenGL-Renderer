srcFiles := $(shell find src/ -name '*.cpp' -or -name '*.c')
objectFiles := bin/glad.o bin/main.o
DEPS := $(wildcard include/*/*.h) $(wildcard src/*.frag) $(wildcard src/*.vert)
cflags := -Wall $(shell pkg-config --cflags glfw3) -Iinclude/
linkerOptions := $(shell pkg-config --static --libs glfw3) -lGL

bin/main.exe: $(objectFiles)
	g++ -o $@ $(objectFiles) $(linkerOptions) && ./$@

$(objectFiles): bin/%.o : src/%.cpp $(DEPS)
	g++ -c $(cflags) $< && mv *.o bin/

clean:
	cd bin/ && rm *.o *.exe