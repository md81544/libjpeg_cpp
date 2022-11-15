.PHONY: debug, clean

test: main.cpp jpeg.h jpeg.cpp
	g++ -O3 -std=c++14 -Wall -Wextra -Wpedantic -Werror -o test *.cpp -ljpeg

debug: main.cpp jpeg.h jpeg.cpp
	g++ -g -O0 -std=c++14 -Wall -Wextra -Wpedantic -Werror -o test *.cpp -ljpeg

clean:
	rm -f test
