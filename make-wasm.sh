emcc -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' gost3411-2012-core.c streebog-wasm-lib.c
