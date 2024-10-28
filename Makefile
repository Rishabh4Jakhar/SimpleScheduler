all:
	@gcc -o test1 prime.c
	@gcc -o test2 fib.c
	@gcc -o test3 factorial.c
	@gcc -o test4 helloworld.c
	@gcc -o shell shell.c -lrt

clean:
	@rm shell test1 test2 test3 test4