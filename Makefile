all: daemon

daemon:
	gcc -o esod daemon.c

clean:
	rm -rf esod
