all:
	gcc -g -o k5_test k5_test.c k5_primitive.c
	gcc -g -o k5_server k5_server.c k5_primitive.c
	gcc -g -o k5_proxy k5_proxy.c  -lk5
clean:
	rm -f k5_test k5_server k5_proxy
