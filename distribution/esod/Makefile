all:
	g++ -std=c++11 -o esod esod.cpp -I/usr/local/ssl/include -L/usr/local/ssl/lib -lcrypto -ldl `mysql_config --cflags --libs`

clean:
	rm -rf esol

debug:
	g++ -g -Wall -Wextra -std=c++11 -o esod esod.cpp -I/usr/local/ssl/include -L/usr/local/ssl/lib -lcrypto -ldl `mysql_config --cflags --libs`
