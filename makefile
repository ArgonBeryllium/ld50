SHELL=/bin/bash

CXX=clang++
CXX_LIBS=-lcumt -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
CXX_FLAGS=--std=c++17 -g

MINGW=x86_64-w64-mingw32-g++
MINGW_LIBS=-lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
MINGW_FLAGS=--std=c++17 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread

test: src/*
	cowsay 'optimise the final build, dipshit'
	$(CXX) src/*.cpp $(CXX_LIBS) $(CXX_FLAGS) -o bin/test

test-win: src/*
	mkdir -p src_w/cumt
	cp src/* src_w/
	cp ~/scripts/cpp/cumt/src/cumt* src_w/cumt/
	cp ~/scripts/cpp/shitrndr/src/shitrndr.h src_w/cumt/
	./bracereplace.sh
	
	$(MINGW) src_w/*.cpp src_w/**/*.cpp $(MINGW_LIBS) $(MINGW_FLAGS) -o bin/win/test.exe
	rm src_w -rf

EMCC_ENV=~/Downloads/installs/emsdk/emsdk_env.sh
EMCC=em++
EMCC_FLAGS= -s WASM=1																							\
			-s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 \
			--preload-file res/																					\
			--std=c++17 -O3
test-web: src/*
	mkdir -p src_w/cumt
	cp src/* src_w/
	cp ~/scripts/cpp/cumt/src/cumt* src_w/cumt/
	cp ~/scripts/cpp/shitrndr/src/shitrndr.h src_w/cumt
	./bracereplace.sh
	source $(EMCC_ENV) && $(EMCC) src_w/*.cpp src_w/cumt/*.cpp $(EMCC_FLAGS) -o bin/index.html
	rm src_w/ -rf
