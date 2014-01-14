all: daemon

daemon:
	g++ -std=c++11 -o esod esod.cpp `mysql_config --cflags --libs`

clean:
	rm -rf esol
