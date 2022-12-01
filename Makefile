CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -I/Users/nugrohodewantoro/Documents/Libraries/tiny_obj -I/Users/nugrohodewantoro/Documents/Libraries/stb_image

Engine: *.cpp src/*/*.cpp src/*/*.hpp
	g++ $(CFLAGS) -o bin/engine.out *.cpp src/*/*.cpp $(LDFLAGS)

.PHONY: test clean

test: Engine
	./bin/engine.out

clean:
	rm -f bin/engine.out