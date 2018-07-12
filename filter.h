#ifndef PARSE_FILTER_H

#define PARSE_FILTER_H

#define ALL_SUBSYS_FILTER_LENGTH 128
#define SINGLE_SUBSYS_FILTER_LENGTH 32
#define MAX_SUBSYS_FILTERS 32

/** It conatin the the process specified in subsys filter */
struct parse_filters_info {
    char subsys_filters[MAX_SUBSYS_FILTERS][SINGLE_SUBSYS_FILTER_LENGTH]; // NULL terminated array of subsys names
    char *white_list_file;
    char *black_list_file;

    int subsys_set; /* booean */
};
int dt; /*!< This hold the divider time for sequence diagram */  
extern int filter_include_subsys(char *line, struct parse_filters_info *parse_filters);
extern int set_subsys_filters(char *from, struct parse_filters_info *parse_filters);

#endif
