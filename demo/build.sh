#!/bin/bash

set -x

c++ -std=c++20 -Wall -Wextra -pedantic -g -I include -c src/App.cpp &
c++ -std=c++20 -Wall -Wextra -pedantic -g -I include -c src/Csv.cpp &
c++ -std=c++20 -Wall -Wextra -pedantic -g -I include -c src/CsvParser.cpp &
c++ -std=c++20 -Wall -Wextra -pedantic -g -I include -c src/main.cpp &
c++ -std=c++20 -Wall -Wextra -pedantic -g -I include -c src/sort.cpp &
wait

c++ -std=c++20 -Wall -Wextra -pedantic -g App.o Csv.o CsvParser.o main.o sort.o -I include -o main 
