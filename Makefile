# Taylor Foxhall
# Tim Hung
# William Jagels

CFLAGS=-g -Wall -Wextra -pedantic -std=gnu11
CXXFLAGS=-g -Wall -Wextra -pedantic -std=c++11
SRCEXT=cpp
HEADEREXT=hpp
LDFLAGS=
SOURCES=$(shell find . -type f -name "*.$(SRCEXT)")
OBJECTS=$(patsubst %.$(SRCEXT),%.o,$(SOURCES))
HEADERS=$(shell find . -type f -name "*.$(HEADEREXT)")
EXECUTABLE=program2
RUN=./$(EXECUTABLE)
RUNV=valgrind --leak-check=full --track-origins=yes $(RUN)
RUNS=perf stat $(RUN)

ccred:=$(shell echo "\033[0;31m")
ccgreen:=$(shell echo "\033[0;92m")
ccyellow:=$(echo -e "\033[0;33m")
ccend:=$(shell echo "\033[0m")


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o:%.$(SRCEXT)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	-rm $(EXECUTABLE) *.o

test: all
	$(RUN)

.PHONY: clean
