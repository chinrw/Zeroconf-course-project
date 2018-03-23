all: main

COMPILER = gcc

main: main.c
	$(COMPILER) main.c -o hw2.out -pthread -ldns_sd
