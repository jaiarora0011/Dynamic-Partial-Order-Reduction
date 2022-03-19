main: main.cpp parse.tab.cpp lex.lex.cpp dpor.cpp
	g++ parse.tab.cpp lex.lex.cpp dpor.cpp main.cpp -ll -lfl -o $@

parse.tab.cpp parse.tab.hpp: parse.y
	bison -o parse.tab.cpp -d parse.y

lex.lex.cpp: lex.l parse.tab.hpp
	flex -o lex.lex.cpp -l lex.l

clean:
	rm -f parse.tab.cpp parse.tab.hpp lex.lex.cpp main