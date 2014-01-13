all: daemon

daemon:
	g++ -std=c++11 -o esol esol.cpp `mysql_config --cflags --libs`

clean:
	rm -rf esod
