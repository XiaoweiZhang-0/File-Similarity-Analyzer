#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cmd.h"
#include <ctype.h>


void showCmd(struct CmdLine cmd) {
    printf("-a %d\n", cmd.analysisThreadCnt);
    printf("-s %s\n", cmd.fileNameSuffix);
    printf("-d %d\n", cmd.dirThreadCnt);
    printf("-f %d\n", cmd.fileThreadCnt);
    printf("files: "); display(cmd.dirQueue);
    printf("dirs: "); display(cmd.fileQueue);
}


struct CmdLine parseCmd(int argc, char *argv[]) {
    struct CmdLine cmdLine = {
        .dirThreadCnt = 1,
        .fileThreadCnt = 1,
        .analysisThreadCnt = 1,
        .fileNameSuffix = ".txt",
        .dirQueue = createQue(),
        .fileQueue = createQue()
    };
   
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
	        struct stat s_buf;
            stat(argv[i], &s_buf);
            if (S_ISREG(s_buf.st_mode) && access(argv[i], F_OK) != -1) {
                enqueue(cmdLine.fileQueue, argv[i]);
            } else if (S_ISDIR(s_buf.st_mode)) {
                enqueue(cmdLine.dirQueue, argv[i]);
            }
        }
    }

    int signal = 0;
    for(int i = 1; i < argc; i++)
    {
        for(int j = 0; j < strlen(argv[i]); j++)
        {
            if(argv[i][j] == 's')
            {
                 
                //  printf("the size of j is %d\n", j);
                //  printf("the size of arg is %lu\n", strlen(argv[i]));
                //  printf(" argv+ is %c\n", argv[i][j+1]);
                if(j+1 >= strlen(argv[i]))
                {
                    // printf("argv is %s\n", argv[i]);
                   cmdLine.fileNameSuffix = " ";
                    signal = 1;
                }
            }
        }
    }
    char opt;
    while ((opt = getopt(argc, argv,"a:f:d:s::")) != -1) {
        switch (opt)
        {
        case 'a':
            //printf("arg is %s\n", optarg);
            if(atoi(optarg) <= 0)
            {
                printf("Invalid analysis thread number\n");
                abort();
            }
            cmdLine.analysisThreadCnt = atoi(optarg);
            break;
        case 's':
            if(signal == 1)
            {
                break;
            }
            else if(optarg[0] == '.')
            {
                cmdLine.fileNameSuffix = optarg;
            }
            else
            {
                printf("arg is %s\n", optarg);
            }
            break;
        case 'd':
            //printf("arg is %s\n", optarg);
            if(atoi(optarg) <= 0)
            {
                printf("Invalid directory thread number\n");
                abort();
            }
            cmdLine.dirThreadCnt = atoi(optarg);
            break;
        case 'f':
           //printf("arg is %s\n", optarg);
            if(atoi(optarg) <= 0)
            {
                printf("Invalid file thread number\n");
                abort();
            }
            cmdLine.fileThreadCnt = atoi(optarg);
            break;
        default:
            //printf("arg is %s\n", optarg);
            puts("only suport -a -s -d -f");
            exit(0);
        }
    }

    return cmdLine;
}
