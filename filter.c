#include "filter_list.h"
#include "filter.h"
#include <string.h>
/* Include or exclude pmanlog from here */
 char filter_list[500][50] = {
             };/*!< This 2D array hold the process of blacklist list file */
 char *include_filter_list[500][150] = {
    };/*!< This 2D array hold the process of white list file */


/** This function take **word1** and **word2** as input and calculate the [levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance)
*/
int distance(const char * word1, int len1,const char * word2, int len2) 
{
    int matrix[len1 + 1][len2 + 1];
    int i;

    for (i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (i = 0; i <= len2; i++) {
        matrix[0][i] = i;
    }

    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = word1[i - 1];
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = word2[j - 1];
            if (c1 == c2) {
                matrix[i][j] = matrix[i - 1][j - 1];
            } else {
                int delete;
                int insert;
                int substitute;
                int minimum;

                delete = matrix[i - 1][j] + 1;
                insert = matrix[i][j - 1] + 1;
                substitute = matrix[i - 1][j - 1] + 1;
                minimum = delete;
                if (insert < minimum) {
                    minimum = insert;
                }
                if (substitute < minimum) {
                    minimum = substitute;
                }
                matrix[i][j] = minimum;
            }
        }
    }
    return matrix[len1][len2];
}

/** This function check if current log line has the process specfied in susbys filter.
If yes, it return 1
else it  return 0
*/
int filter_include_subsys(char * line, struct parse_filters_info * parse_filters) {
    char( * cp)[SINGLE_SUBSYS_FILTER_LENGTH] = parse_filters -> subsys_filters;
    int i = 0;
    char * subsys;
    /* If not subsys filter set include all */
    if (!parse_filters -> subsys_set) {
        return 1;
    }

    /* Trace only those subsys which are set */
    subsys = (char * ) cp;
    while ((subsys[0] != '\0') && (i < MAX_SUBSYS_FILTERS)) {
        //fprintf(stderr,"subsys to compare: %s line: %s\n",subsys, line);
        if (strstr(line, subsys)) return 1;
        i++;
        cp++;
        subsys = (char * ) cp;
    }

    /* exclude trace */
    //fprintf(stderr,"subsys %s not found in: %s\n",cp, line);
    return 0;
}

/** This function is used to peopulate the array of subsys filter*/
int set_subsys_filters(char * from, struct parse_filters_info * parse_filters) {

    ///This function set the subsys filter.

    char * token;
    int i = 0;
    char local_from[SINGLE_SUBSYS_FILTER_LENGTH];

    strcpy(local_from, from);

    token = strtok(local_from, ",");
    if (!token) {
        return -1;
    }
    while (token && (i < MAX_SUBSYS_FILTERS)) {
        strcpy(parse_filters -> subsys_filters[i], token);
        token = strtok(NULL, ",");
        //fprintf(stderr,"Token (%d): %s\n",i,parse_filters -> subsys_filters[i]);
        i++;
    }
    parse_filters -> subsys_set = 1;
    return 0;
}

/*-----------*/
