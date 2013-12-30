all: daemon

daemon:
	g++ -std=c++11 -o esod daemon.cpp `mysql_config --cflags --libs`

clean:
	rm -rf esod
