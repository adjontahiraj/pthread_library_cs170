all: sample_grader

sample_grader:autograder_main.cpp thread_lib 
	g++ autograder_main.cpp threads.o -o sample_grader

thread_lib:threads.cpp
	g++ -c threads.cpp -o threads.o
 
 
