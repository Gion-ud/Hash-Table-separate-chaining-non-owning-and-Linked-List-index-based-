all: list.o main

CFLAGS = -O2 -Wall -Wextra -fno-exceptions -fno-strict-aliasing -Wno-maybe-uninitialized

list.o: src/list.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

main: src/testls.c build/list.o | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude

clean:
	rm build/*o
