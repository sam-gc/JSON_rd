json_test: main.c json.c
	gcc -o json main.c json.c arraylist.c hashtable.c mempool.c -Wall -g

clean:
	rm json
