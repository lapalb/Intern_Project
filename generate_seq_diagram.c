#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "filter_list.h"
#include "filter.h"
/*-----------*/

int is_in_include_list(char * full_log, int log_len) {
    int i, d;
    for (i = 0; i<wl; i++) {
        d = distance(full_log, log_len, include_filter_list[i], strlen(include_filter_list[i]));
        //fprintf(stderr,"Distance between ==%s== and ==%s== is: %d\n",full_log,include_filter_list[i],d);
        if (d <= INCLUDE_DISTANCE) {
            return 1;
        }
    }
    return 0;
}

/*-----------*/

/* 
 * 2018/04/05 14:10:00.112 L316 {cmand_R0-0}{1} [cmand] 17680 :: [ACTIVE]  CPLD sfchassis: get vector starfleet
 */
/** This function pareses a log line and extract paramter like log time, date, process name and pid*/
int parse_line_from_all_text(int source_line_no, char * line, char * line_no, char * date, char * * tag, char * log_time, char * proc, char * proct, int * pid, char * log) {
    char * cp = line;
    int r, colon_count, i;
    char cpid[PID_LENGTH];
    while ( * cp && !isdigit( * cp)) cp++;
    if (!cp) return -1;
    r = sscanf(line, "%s %s %s %s %s %s", date, log_time, line_no, proc, proct, cpid);
    if (r != 6) {
        fprintf(stderr, "Error in reading: Line: %d Line:  %s", source_line_no, line);
        return -1;
    }

    cp = strchr(proc, ':');
    if (cp) * cp = '\0';

    * pid = atoi(cpid);

    cp = strstr(line, "::");
    cp++;
    cp++;

    if ((cp - line) > LINE_LENGTH) return -1;

    strcpy(log, cp);

    if (strstr(line, "ACTIVE")) { * tag = "ACTIVE";
    }
    if (strstr(line, "STDBY")) { * tag = "STDBY";
    } else *tag = " ";

    return 0;
} /*-----------*/

int generate_seq_diagram(int check_include_list) {
    FILE * fp, * seq_fp;
    char line[10000];
    char line_no[LINE_LENGTH];
    char date[DATE_LENGTH], log_time[TIME_LENGTH];
    char log[10000];
    char proc_name_1[PROC_NAME_1];
    char proc_name_2[PROC_NAME_2];
    int pid;
    char * cp;
    char * tag;
    int source_line_no = 0, excluded = 0;

    fp = fopen(ALL_TXT, "r");
    if (!fp){
        fprintf(stderr, "Could not open %s\n", ALL_TXT);
        return -1;
    }

    if (!(seq_fp = seq_uml_prologue())) {
        return -1;
    }
    int ad = 0;
    while (fgets(line, sizeof(line), fp)) {
        source_line_no++;
        parse_line_from_all_text(source_line_no, line, line_no, date, & tag, log_time, proc_name_1, proc_name_2, & pid, log);
        if (check_include_list) {
            if (is_in_include_list(log, strlen(log))) {
                cp = strchr(log, '\n');
                if (cp) * cp = '\0';
                ad++;
                add_seq_log_entry(seq_fp, date, log_time, tag, proc_name_1, proc_name_2, pid, log, dt);
            } else {
                //fprintf(stderr,"Excluded: Line# %d Line: %s\n",source_line_no,line); 
                excluded++;
            }
        } else {
            cp = strchr(log, '\n');
            if (cp) * cp = '\0';
            ad++;
            add_seq_log_entry(seq_fp, date, log_time, tag, proc_name_1, proc_name_2, pid, log, dt);
        }
    }

    fclose(fp);
    seq_uml_epilogue(seq_fp);
    fprintf(stderr, "Total excluded from all logs: %d  %d    %d    \n", excluded, ad, source_line_no);
}

/*--------------*/

int main(int argc, char * argv[]) {
    dt = strtol(argv[1], NULL, 10);
    if ((argc == 2) && (strcmp(argv[1], "-i") == 0)) {
        generate_seq_diagram(1);
    } else {
        generate_seq_diagram(0);
    }
    return 0;
}

/*--------------*/
