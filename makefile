all: list.o liblist.dll main

CFLAGS = -O2 -Wall -Wextra -fno-exceptions -fno-strict-aliasing -Wno-maybe-uninitialized

list.o: src/list.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

liblist.dll: build/list.o | bin
	cc -shared $< -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def

main: src/testls.c lib/liblist.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -llist

clean:
	rm build/*o
