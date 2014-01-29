all:
	# Compile library Java files.
	javac EsoLocal/*.java
	# Create JNI headers.
	javah -d EsoLocal EsoLocal.EsoLocal
	# Create library folder
	mkdir -p lib
	# Create shared library.
	g++ -std=c++11 -o lib/libesol.so -shared -I$JAVA_HOME EsoLocal/EsoLocal_EsoLocal.cpp -lc -fPIC
	# Create jar file.
	jar -cvf lib/EsoLocal.jar EsoLocal/*.class
	# Compile Java examples.
	javac -classpath lib/EsoLocal.jar *.java

debug:
	# Compile library Java files.
	javac EsoLocal/*.java
	# Create JNI headers.
	javah -d EsoLocal EsoLocal.EsoLocal
	# Create library folder
	mkdir -p lib
	# Create shared library. Some extra warning are added.
	g++ -g -Wall -Wextra -std=c++11 -o lib/libesol.so -shared -I$JAVA_HOME EsoLocal/EsoLocal_EsoLocal.cpp -lc -fPIC
	# Create jar file.
	jar -cvf lib/EsoLocal.jar EsoLocal/*.class
	# Compile Java examples.
	javac -classpath lib/EsoLocal.jar *.java

clean:
