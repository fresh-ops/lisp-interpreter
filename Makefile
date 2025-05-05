all: libparser.a main.o
	gcc main.o -Llibs -lparser -o ./app
	rm *.o
main.o:
	gcc -c main.c
libparser.a:
	gcc -c ./parser/*.c
	ar rcs libparser.a *.o
	mv libparser.a libs
	rm *.o