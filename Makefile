all:
	gcc -o bign main.c

test: all
	./bign < test_case.txt
