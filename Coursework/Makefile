all: coursework
coursework: coursework.o search_roots_lib.o find_def_integral_lib.o
	gcc -o coursework coursework.o search_roots_lib.o find_def_integral_lib.o -lm
coursework.o: coursework.c search_roots_lib.h find_def_integral_lib.h types.h
	gcc -c -o coursework.o coursework.c
find_def_integral_lib.o: find_def_integral_lib.c types.h
	gcc -c -o find_def_integral_lib.o find_def_integral_lib.c
search_roots_lib.o: search_roots_lib.c types.h
	gcc -c -o search_roots_lib.o search_roots_lib.c
clean:
	del *.o