CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread

Engine: *.cpp */*.cpp */*.hpp
	g++ $(CFLAGS) -o bin/engine.out *.cpp */*.cpp $(LDFLAGS)

.PHONY: test clean

test: Engine
	./bin/engine.out

clean:
	rm -f bin/engine.out