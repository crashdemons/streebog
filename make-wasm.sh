rm -rf streebog-wasm-dist
mkdir streebog-wasm-dist
emcc -O3 -s MODULARIZE=1 -s -s 'EXPORT_NAME="createStreebogModule"' -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' gost3411-2012-core.c streebog-wasm-lib.c -o streebog-wasm-dist/streebog-wasm.js

cat streebog-wasm-dist/streebog-wasm.js streebog-js-wrapper/streebog-js.js > streebog-wasm-dist/streebog-wrapped.js
