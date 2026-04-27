all: main.o lodepng.o
	gcc main.o lodepng.o -o demo.exe
main.o: main.c
	gcc main.c -c
lodepng.o: lodepng.c
	gcc lodepng.c -c
debug: main-debug.o lodepng-debug.o
	gcc -g main-debug.o lodepng-debug.o -o demo-debug.exe
main-debug.o: main.c
	gcc main.c -g -c -o main-debug.o
lodepng-debug.o: lodepng.c
	gcc lodepng.c -g -c -o lodepng-debug.o
clean:
	rm *.o
	rm demo.exe