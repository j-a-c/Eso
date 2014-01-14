all: daemon

daemon:
	g++ -std=c++11 -o esoca esoca.cpp `mysql_config --cflags --libs`

clean:
	rm -rf esoca
