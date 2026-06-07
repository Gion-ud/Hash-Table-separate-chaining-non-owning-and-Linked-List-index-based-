#all: ht.o libht.dll testht
#all: list.o liblist.dll testls
all: kvarena.o kvfile.o tskva

CFLAGS = \
	-O2 -Wall -Wextra -fno-exceptions -fno-strict-aliasing -D_DEBUG
#	-Wno-maybe-uninitialized \
#	-Wno-unused-function

list.o: src/list.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

liblist.dll: build/list.o | bin
	cc -shared $< -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def

testls: tests/testls.c lib/liblist.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -llist

ht.o: src/hash_table.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

libht.dll: build/ht.o | bin
	cc -shared $< -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def

testht: tests/testht.c lib/libht.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -lht

kvarena.o: src/kvarena.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

kvfile.o: src/kvfile.c | build
	cc -c $< -o build/$@ $(CFLAGS) -Iinclude

tskva: tests/tskva.c build/kvarena.o build/kvfile.o | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -lmman -lz

clean:
	rm build/*o
