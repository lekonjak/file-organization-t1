all: compile
	
compile:
	gcc -o programaTrab1 src/*.c 
run:
	./programaTrab1
dcompile:
	gcc -o programaTrab1 main.c src/*.c -D DEBUG -g -Wall -Wextra
drun:
	valgrind ./programaTrab1
