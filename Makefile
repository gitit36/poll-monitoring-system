
output: main.o tokenize.o
	g++ main.o tokenize.o -o output 

main.o: main.cpp
	g++ -c main.cpp

tokenize.o: tokenize.cpp tokenize.h
	g++ -c tokenize.cpp

clean:
	rm *.o output