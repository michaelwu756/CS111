INPUT=file1
OUTPUT=file2

all: lab0

lab0: lab0.c
	gcc -Wall -Wextra -o lab0 lab0.c

.PHONY: check clean dist
check: lab0
	@echo "Testing default copying with no arguments:"
	@echo "ayylmao">$(INPUT)
	@./lab0 <$(INPUT) >$(OUTPUT); \
	cmp $(INPUT) $(OUTPUT); \
	if [ $$? -eq 0 ]; then \
		echo "Passed"; \
	else \
		echo "Failed"; \
	fi
	@echo
	@echo "Testing copying with --input option and --output option:"
	@echo "ayylmao2">$(INPUT)
	@./lab0 --input=$(INPUT) --output=$(OUTPUT); \
        cmp $(INPUT) $(OUTPUT); \
        if [ $$? -eq 0 ]; then \
                echo "Passed"; \
        else \
                echo "Failed"; \
        fi
	@echo
	@echo "Testing segfault option:"
	@exec 2> /dev/null; \
	./lab0 --segfault; \
	if [ $$? -eq 139 ]; then \
                echo "Passed"; \
        else \
                echo "Failed"; \
        fi
	@echo
	@echo "Testing segfault is catched:"
	@exec 2> /dev/null; \
        ./lab0 --segfault --catch; \
        if [ $$? -eq 4 ]; then \
                echo "Passed"; \
        else \
                echo "Failed"; \
        fi
	@echo
	@echo "Testing unrecognized argument:"
	@exec 2> /dev/null; \
	./lab0 --ayylmao; \
	if [ $$? -eq 1 ]; then \
                echo "Passed"; \
        else \
                echo "Failed"; \
        fi
	@rm $(INPUT) $(OUTPUT)

clean:
	rm -f lab0 $(INPUT) $(OUTPUT) lab0-404751542.tar.gz

dist: lab0.c Makefile backtrace.png breakpoint.png README
	tar -czf lab0-404751542.tar.gz $^
