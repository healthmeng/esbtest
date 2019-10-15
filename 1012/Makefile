all:k5test k5_test_loop

k5test:k5_test.c
	gcc -I../include -lk5 k5_test.c -g -o k5test

k5_test_loop:k5_test_loop.c
	gcc -I../include -lk5 k5_test_loop.c -g -o k5_test_loop

clean:
	rm -f k5test k5_test_loop
