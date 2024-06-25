all: HTTPSimple

clean:
	@rm -f *.o
	@rm -f HTTPSimple

HTTPSimple: main.o auth.o
	gcc -o HTTPSimple $^ -lssl -lcrypto -lpam -lpam_misc

main.o: main.c auth.h
	gcc -c -o main.o main.c

auth.o: auth.c auth.h
	gcc -c -o auth.o auth.c
