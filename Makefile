#	gcc -D GENERATE_INCLUDE_LOGS -g filter.c log2uml.c parse_log.c -o ../../parse_log

insight: insight.c filter.c filter.h filter_list.h generate_seq_diagram.c filter.c log2uml.c
	gcc -g -w filter.c insight.c -o insight -std=c11 

seq_diag:  generate_seq_diagram.c filter.c log2uml.c filter_list.h
	gcc -g -w filter.c  log2uml.c generate_seq_diagram.c -o seq_diag -std=c11

all: insight  seq_diag
