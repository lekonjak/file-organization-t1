EXECUTABLE = programaTrab1

REGCOUNT = `cat ./src/input.csv | wc -l`
REGLAST = $$(( $(REGCOUNT) - 1 ))
REGSEQ = $(shell seq 0 ${REGLAST})


compile:
	gcc -o $(EXECUTABLE) src/*.c
run:
	./$(EXECUTABLE)
dcompile:
	gcc -o $(EXECUTABLE) src/*.c -D DEBUG -g -Wall -Wextra
drun:
	valgrind ./$(EXECUTABLE)
test:
	$(MAKE) test_setup
	@echo "\nStarting tests..."
	./$(EXECUTABLE) 1 ./src/input.csv
	./$(EXECUTABLE) 2 >> test_log/full.output
	@echo "Executing $(REGCOUNT) gets..."
	@$(foreach var,$(REGSEQ), ./$(EXECUTABLE) 4 $(var) >> test_log/single.output;)
# Agora dá um diff nos dois arquivos gerados

test_setup:
ifeq (,$(wildcard test_log))
	mkdir test_log
else
	rm -rf test_log/* 
endif