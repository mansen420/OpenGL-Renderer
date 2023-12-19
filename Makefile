srcFiles := $(shell find src/ -name '*.cpp')
objectFiles := bin/main.o
DEPS := $(wildcard include/*/*.h) $(wildcard include/*.h) $(wildcard src/*.frag) $(wildcard src/*.vert)
cflags := -Wall $(shell pkg-config --cflags glfw3) -Iinclude/
linkerOptions := $(shell pkg-config --static --libs glfw3) -lGL

bin/main.exe: $(objectFiles) bin/glad.o
	g++ -o $@ $(objectFiles) bin/glad.o $(linkerOptions) && ./$@

$(objectFiles): bin/%.o : src/%.cpp $(DEPS)
	g++ -c $(cflags) $< && mv *.o bin/

bin/glad.o: src/glad.c 	#special rule for glad.c
	g++ -c $(cflags) src/glad.c && mv glad.o bin/

clean:
	cd bin/ && rm *.o *.exe
run:
	./bin/main.exe