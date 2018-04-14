all: a b 

a: miProxy.h miProxy.cpp
	g++ -o miProxy -std=c++11 miProxy.cpp

b: miProxy.h nameserver.cpp
	g++ -o nameserver -std=c++11 nameserver.cpp

clean: miProxy
	rm -fr miProxy
	rm -fr nameserver
