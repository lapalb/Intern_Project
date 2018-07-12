#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "filter_list.h"

/*---------*/
/** This write the header of dot file so that it can be identified by plantuml*/
FILE * seq_uml_prologue() {
    FILE * fp;
    fp = fopen(UML_FILE_NAME, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create %s file\n", UML_FILE_NAME);
        return (FILE * ) 0;
    }
    fprintf(fp, "@startuml\n");
    return fp;
}

/*---------*/
/** This write the footer of dot file so that it can be identified by plantuml*/
int seq_uml_epilogue(FILE * fp) {
    fprintf(fp, "@enduml\n");
    fclose(fp);
}


/** Replace unacceptable plant uml chars */
void replace_chars(char * str) {
    char * cp = str;
    if (!cp) return;

    while ( * cp) {
        switch ( * cp) {
        case '/':
        case '{':
        case '}':
        case '-':
            * cp = '_';
            break;
        case '[':
        case ']':
        case '%':
            * cp = '_';
            break;
        default:
            break;
        }
        cp++;
    }
}

/** Return the time difference between two time */
int td(char * prev, char * cur) {

    //Getting the TimeStamp of  Previous Log
    char prev_local[15];
    strcpy(prev_local, prev);
    int log_timei = 0; //Holds the Timestamp of previous time
    char * ptr = strtok(prev_local, ":.");
    int i = 1;
    while (ptr != NULL) {
        if (i <= 3600) {
            log_timei += atoi(ptr) * 3600 * 1000 / i;
            ptr = strtok(NULL, ":.");
            i *= 60;
        } else {
            log_timei += atoi(ptr);
            ptr = strtok(NULL, ":.");
        }
    }
    //Getting the TimeSatmp of current log
    char strl[15];
    strcpy(strl, cur);
    int stri = 0; // Hold the Timestamp of current time
    ptr = strtok(strl, ":.");
    int j = 1;
    while (ptr != NULL) {
        if (j <= 3600) {
            stri += atoi(ptr) * 3600 * 1000 / j;
            ptr = strtok(NULL, ":.");
            j *= 60;
        } else {
            stri += atoi(ptr);
            ptr = strtok(NULL, ":.");
        }
    }
    return abs(stri - log_timei);
}

/*---------*/
#define TIME_COLOR "#FF0000" /*< Time is denoted in Red color*/
#define PROC_COLOR "#FF8833" /*< Process is denoted in Orange*/
#define LOG_COLOR "#0000FF" /*< Log is denoted in blue color*/
#define SEQUENCE_COLOR "#000000"/*< Sequence Color is Black*/
#define ACT_STD_COLOR "#23AB67"/*< Standdar color is lime green*/
#define ERROR_LOG_COLOR "#FF0000"/*< Error log is Redd*/
#define WARNING_LOG_COLOR "#FFAA00"/*< Warning log is yellow*/
char prev[15];

/** This function is used to insert a log in dot file. It take a filename and other paramter of log line */
int add_seq_log_entry(FILE * fp, char * date, char * log_time,
    char * tag,
    char * proc_name_1,
    char * proc_name_2, int pid,
    char * log, int dt) {
    /* Running sequence number to help track the events while browsing the web page */
    static int seq_no;
    char * log_color;
    replace_chars(proc_name_1);
    replace_chars(proc_name_2);
    replace_chars(log_time);

    /* Show it as self loop, proc_name_1 to proc_name_1 */
    if (strcasestr(log, "err ") || strcasestr(log, "error") || strcasestr(log, "fail")) {
        log_color = ERROR_LOG_COLOR;
    } 
    else if (strcasestr(log, "warning")) {
        log_color = WARNING_LOG_COLOR;
    } 
    else {
        log_color = LOG_COLOR;
    }

    if (td(prev, log_time) > dt) {
        fprintf(fp, "== Delay By %d Mili_Second ==\n", dt);
    }
    int log_length = strlen(log);
    
    if (log_length > 90) {
        int i = 0;
        for (i = 80; i < log_length; i = i + 80) {
            log[i] = '\\';
            log[i + 1] = '\\n';
        }
    }
    fprintf(fp, "%s%s -> %s%s : <color %s>%s</color> [<color %s>**%d**</color>] [<color %s>%s</color>] <color %s>%s</color>\n",
        tag, proc_name_1, tag, proc_name_1,
        TIME_COLOR, log_time,
        SEQUENCE_COLOR, seq_no,
        PROC_COLOR, proc_name_2,
        log_color, log);

    seq_no++;
    strcpy(prev, log_time);
    return 0;
}
