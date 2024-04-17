CXX = c++ -std=c++20
CFLAGS = -Wall -Wextra -pedantic -g
INCLUDE = -I include
SOURCES = App.cpp \
		  Csv.cpp \
		  CsvParser.cpp \
		  main.cpp \
		  sort.cpp

OBJECTS := $(SOURCES:.cpp=.o)

VPATH = src:

main: $(OBJECTS)
	$(CXX) $(CFLAGS) $^ $(INCLUDE) -o $@ 

$(OBJECTS): %.o : %.cpp

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< 

clean:
	rm -f *.o main
