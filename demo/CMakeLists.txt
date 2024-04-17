cmake_minimum_required(VERSION 3.17)
project(sorts)

set(CMAKE_CXX_STANDARD 20)

include_directories("include")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -g")
set(CMAKE_SIURCE_DIR src)

add_executable(main
	src/App.cpp
	src/Csv.cpp
	src/CsvParser.cpp
	src/main.cpp
	src/sort.cpp
)

