all:
	cp script.js build
	emcc main.c build/src/libt9.a -o build/index.html --shell-file index.html -I../src -sEXPORTED_FUNCTIONS=_get_word,_malloc,_free,_main -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sSTACK_SIZE=655360000

build_lib:
	emcmake cmake -B build/ -S ../ -DBUILD_LIB=ON
	cd build && make -j$(shell nproc)

clean:
	rm -r build
