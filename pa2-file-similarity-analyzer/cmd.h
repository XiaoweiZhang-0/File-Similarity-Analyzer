#ifndef CMD_H

#include "queue.h"

/**
Parameter        Option ArgumentType      Default
DirectoryThreads -dN    Positive_integer  1
FileThreads      -fN    Positive_integer  1
AnalysisThreads  -aN    Positive_integer  1
FileNameSuffix   -sS    String            ".txt"
*/

struct CmdLine {
    int dirThreadCnt;
    int fileThreadCnt;
    int analysisThreadCnt;
    char *fileNameSuffix;

    queue_t *dirQueue;
    queue_t *fileQueue;
};

void showCmd(struct CmdLine cmd);
struct CmdLine parseCmd(int argc, char *argv[]);

#endif