build/main: main.c build/str.o build/dynamic_array.o 
	gcc -I. -Wall -Wextra -ggdb -o build/main main.c build/str.o build/dynamic_array.o

build/dynamic_array.o: dynamic_array.c dynamic_array.h
	gcc -Wall -Wextra -ggdb -c -o build/dynamic_array.o dynamic_array.c

build/str.o: str.c str.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/str.o str.c

clean:
	rm -rf build/
