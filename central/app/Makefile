all:
	# Include Python, OpenSSL, MySql
	g++ databaseModule.cpp -o databaseModule.so -std=c++11 -shared -I/usr/include/python2.7/ -lpython2.7 -I/usr/local/ssl/include -L/usr/local/ssl/lib -lcrypto -ldl `mysql_config --cflags --libs`

