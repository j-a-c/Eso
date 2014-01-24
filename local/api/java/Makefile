all: java

java:
	javac *.java
	javah EsoLocal

	g++ -std=c++11 -o libesol.so -shared -I$JAVA_HOME EsoLocal.cpp -lc -fPIC
clean:
	
