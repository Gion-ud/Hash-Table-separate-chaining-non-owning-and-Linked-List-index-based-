all: ht.o libht.dll testht
#all: list.o liblist.dll testls

CFLAGS = -O2 -Wall -Wextra -fno-exceptions -fno-strict-aliasing 
# -D_DEBUG
# -Wno-maybe-uninitialized

list.o: src/list.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

liblist.dll: build/list.o | bin
	cc -shared $< -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def

testls: src/testls.c lib/liblist.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -llist

ht.o: src/hash_table.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

libht.dll: build/ht.o | bin
	cc -shared $< -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def

testht: src/testht.c lib/libht.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -lht

clean:
	rm build/*o
