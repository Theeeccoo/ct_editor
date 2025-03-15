build/main: main.c build/str.o build/dynamic_array.o build/utils.o
	gcc -I. -Wall -Wextra -ggdb main.c -o build/main build/str.o build/dynamic_array.o build/utils.o

build/dynamic_array.o: dynamic_array.c dynamic_array.h utils.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/dynamic_array.o dynamic_array.c

build/utils.o: utils.c utils.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/utils.o utils.c

build/str.o: str.c str.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/str.o str.c

clean:
	rm -rf build/
