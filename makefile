all:	compile
	
compile:
	gcc -o programaTrab1 main.c build/*
run:
	./programaTrab1
dcompile:
	gcc -o programaTrab1 main.c build/* -D DEBUG
drun:
	valgrind ./programaTrab1
