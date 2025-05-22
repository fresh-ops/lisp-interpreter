all: libparser.a libevaluator.a main.o
	gcc main.o -Llibs -lparser -levaluator -o ./app
	rm *.o
main.o:
	gcc -c main.c
libparser.a:
	gcc -c ./parser/*.c
	ar rcs libparser.a *.o
	mv libparser.a libs
	rm *.o
libevaluator.a:
	gcc -c ./evaluator/*.c
	ar rcs libevaluator.a *.o
	mv libevaluator.a libs
	rm *.o