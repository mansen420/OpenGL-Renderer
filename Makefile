srcFiles := $(shell find src/ -name '*.cpp')
objectFiles := bin/main.o
DEPS := $(wildcard include/*/*.h) $(wildcard include/*.h) $(wildcard src/*.frag) $(wildcard src/*.vert)
cflags := -Wall $(shell pkg-config --cflags glfw3) $(shell pkg-config --cflags assimp) -Iinclude/
linkerOptions := $(shell pkg-config --static --libs glfw3) $(shell pkg-config --static --libs assimp) 

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

#how I linked with assimp : for future reference
#	1. I cloned and built the project
#	2. I copied the assimp.pc file to usr/local/lib/pkconfig
#	3. I specifeid the pkg-config path in my .bashrc
#	4. I included pkg-config --cflags --libs Assimp in my makefile
#	5. I specified the LD_LIBRARY_PATH to usr/local/lib in my .bashrc
#	6. I moved libassimp.so.5 to the library path
#	7. profit