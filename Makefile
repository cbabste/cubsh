cubsh: cubsh.o
	gcc -o cubsh cubsh.o

cubsh.o: cubsh.c
	gcc -c cubsh.c -o cubsh.o

clean:
	rm -f cubsh *.o