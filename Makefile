# Taylor Foxhall
# Tim Hung
# William Jagels

CFLAGS=-g -Wall -Wextra -pedantic -std=gnu11
CXXFLAGS=-g -Wall -Wextra -pedantic -std=gnu++11 -DNDEBUG
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
	-rm $(EXECUTABLE) *.o *.d

test: all initialize
	@echo -e "$(ccgreen)Testing small file$(ccend)"
	$(RUN) ./test/import_small
	@echo -e "$(ccgreen)Testing large file$(ccend)"
	$(RUN) ./test/import_large
	@echo -e "$(ccgreen)Testing overwrite without new block$(ccend)"
	$(RUN) ./test/overwrite_small
	@echo -e "$(ccgreen)Testing overwrite needing new block$(ccend)"
	$(RUN) ./test/overwrite_large

initialize: all
	$(RUN) initialize

lint: $(SOURCES) $(HEADERS)
	cpplint $^


%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(SOURCES:%.cpp=%.d)

.PHONY: clean
