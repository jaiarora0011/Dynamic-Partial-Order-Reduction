FINAL_EXEC=dpor
IDIR=include
SRC=src
CC=g++
CFLAGS=-I$(IDIR) -ll -lfl
LEX=lex
PARSE=parse
OUTPUT=output
INPUT=input

DEPS=$(wildcard $(IDIR)/*.hpp)
SOURCES=$(SRC)/dpor.cpp $(SRC)/main.cpp
PARSE_SOURCE=$(SRC)/$(PARSE).tab.cpp
LEX_SOURCE=$(SRC)/$(LEX).lex.cpp

TESTS=$(wildcard $(INPUT)/*.txt)
DOTS=$(patsubst $(INPUT)/%.txt, $(OUTPUT)/%.dot, $(TESTS))

.PHONY: compile

compile: $(SOURCES) $(PARSE_SOURCE) $(LEX_SOURCE) $(DEPS)
	g++ $(PARSE_SOURCE) $(LEX_SOURCE) $(SOURCES) $(CFLAGS) -o $(FINAL_EXEC)

$(PARSE_SOURCE): $(SRC)/$(PARSE).y
	bison -o $@ -d $^

$(LEX_SOURCE): $(SRC)/$(LEX).l $(SRC)/$(PARSE).tab.hpp
	flex -o $@ -l $<

clean:
	cd $(OUTPUT) && rm -f *.dot *.log *.aux *.pdf *.tex

clean_all: clean
	rm -f $(FINAL_EXEC)
	cd $(SRC) && rm -f *.tab.cpp *.lex.cpp *.tab.hpp

.PHONY: test 
test: $(DOTS)

$(OUTPUT)/%.dot: $(INPUT)/%.txt $(FINAL_EXEC)
	./$(FINAL_EXEC) $< $@
	dot2tex $@ > $@.tex 
	cd $(OUTPUT) && pdflatex $*.dot.tex > /dev/null