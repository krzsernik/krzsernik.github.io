emcc k20.cpp -s WASM=1 --js-opts 1 --llvm-opts 3 --pre-js pre.js \
	--preload-file emcc_assets/@. -o krzsernik.github.io/K20/k20.js \
	-s ALLOW_MEMORY_GROWTH=1 --llvm-lto 3 \
	-s EXPORTED_FUNCTIONS='["_main", "_setImage", "_buildLogo", "_getLastError"]' \
	-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall"]' -v -s LZ4=1
