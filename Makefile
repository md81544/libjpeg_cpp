.PHONY: debug, clean

INC :=

OS := $(shell uname)
ifeq ($(OS),Darwin)
	INC := -I $(shell brew --prefix)/include
endif

test: main.cpp jpeg.h jpeg.cpp
	g++ -O3 -std=c++14 -Wall -Wextra -Wpedantic -Werror $(INC) -o test *.cpp -ljpeg

debug: main.cpp jpeg.h jpeg.cpp
	g++ -g -O0 -std=c++14 -Wall -Wextra -Wpedantic -Werror $(INC) -o test *.cpp -ljpeg

clean:
	rm -f test
