all: \
	kvht.o kvarena.o kvfile.o kvimg.o kvtbl.o \
	libkv.dll libkvtbl.dll \
	tskv \
	add-bin-to-path

CFLAGS = \
	-O2 -Wall -Wextra -fno-exceptions -fno-strict-aliasing -D_DEBUG -g # -fsanitize=address
#	-O0
#	-Wno-maybe-uninitialized \
#	-Wno-unused-function

LIBKV_CFLAGS = -D_BUILD_LIBKV_SHARED -fvisibility=hidden

kvht.o: src/kvht.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(LIBKV_CFLAGS) -Iinclude -Isrc

kvarena.o: src/kvarena.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(LIBKV_CFLAGS) -Iinclude -Isrc

kvfile.o: src/kvfile.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(LIBKV_CFLAGS) -Iinclude -Isrc

kvimg.o: src/kvimg.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(LIBKV_CFLAGS) -Iinclude -Isrc

kvtbl.o: src/kvtbl.c | build
	cc -c $< -o build/$@ $(CFLAGS) $(LIBKV_CFLAGS) -Iinclude -Isrc


LIBKV_CFLAGS = -D_BUILD_LIBKV_SHARED -fvisibility=hidden

LIBKV_OBJ = build/kvht.o build/kvarena.o build/kvfile.o build/kvimg.o
LIBKVTBL_OBJ = build/kvtbl.o

bin lib:
	mkdir -p $@

libkv.dll: $(LIBKV_OBJ) | bin lib
	cc -shared $^ -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def \
		-Llib -lmman -lz

libkvtbl.dll: $(LIBKVTBL_OBJ) lib/libkv.dll.a | bin lib
	cc -shared $^ -o bin/$@ $(CFLAGS) \
		-Wl,--kill-at \
		-Wl,--out-implib,lib/$@.a \
		-Wl,--output-def,lib/$@.def \
		-Llib -lkv

tskv: tests/tskv.c lib/libkv.dll.a | bin
	cc $^ -o bin/$@ $(CFLAGS) -Iinclude -Llib -lkv -lkvtbl

add-bin-to-path:
	export PATH="$$PATH:$$(pwd)/bin"

clean:
	rm build/*o
