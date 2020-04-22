a.out: tester.o node.o string_set.o
	g++ tester.o node.o string_set.o -o ss -g
tester.o: node.h string_set.h tester.cpp
	g++ -std=c++0x -c tester.cpp -g

node.o: node.cpp node.h
	g++ -c node.cpp -g

string_set.o: string_set.h node.h string_set.cpp
	g++ -c string_set.cpp -g

clean:
	rm -f tester.o node.o string_set.o a.out
