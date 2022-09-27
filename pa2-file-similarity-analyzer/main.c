#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cmd.h"
#include "queue.h"
#include "hash.h"
#include "jsd.h"


struct DirThreadArgv {
    char *sufix;
    char **dirs;
    int dirCount;
    queue_t *fileQue;
};


struct FileThreadArgv {
    char **files;
    int fcount;
    queue_t *wfdQue;
};


struct WFDInfo {
    char *file;
    struct HashTable *ht;
};


struct AnalysisTask {
    struct WFDInfo *wfdA;
    struct WFDInfo *wfdB;
    double jsdVal;
};


struct AnalysisThreadArgv {
    struct AnalysisTask *tasks;
    int start;
    int len;
};


int endWith(const char *str, const char *sufix) {
    //printf("suffix is %s, file ending is %s\n", sufix, str);
    if(strcmp(sufix, " "))
    {
        sufix = "";
    }
    // for(int i=0; i< strlen(str); i++)
    // {
    //     if(str[i] == '.')
    //     {
    //         break;
    //     }
    // }
    int i = strlen(str) - 1;
    int j = strlen(sufix) - 1;
    if (j > i) {
        return 0;
    }
    for ( ; str[i] == sufix[j]; i--, j--);
    return j < 0;
}



void displayWfd(queue_t *que) {
    printf("size=%d {", que->size);
    struct QueNode *cur = que->head->next;
    while (cur) {
        struct WFDInfo *wfdInfo = (struct WFDInfo *) cur->data;
        puts(wfdInfo->file);
        displyHashTable(wfdInfo->ht);
        cur = cur->next;
    }
    printf("}\n");
}


void traverseDir(char *dir, char *sufix, queue_t *que) {
    struct stat s_buf;
    stat(dir, &s_buf);
    if (!S_ISDIR(s_buf.st_mode) || access(dir, F_OK) != 0) {
        return;
    }

    struct dirent *dirEnt = NULL;
    DIR *dirp = opendir(dir);
    while ((dirEnt = readdir(dirp)) != NULL) {
		if (dirEnt->d_type == DT_DIR && strcmp(dirEnt->d_name, ".") != 0 && strcmp(dirEnt->d_name, "..") != 0) {
            char fullPath[2000];
            sprintf(fullPath, "%s/%s", dir, dirEnt->d_name);
            traverseDir(fullPath, sufix, que);
		} else if (dirEnt->d_type == DT_REG) {
            if (endWith(dirEnt->d_name, sufix) == 1) {
                char *fullPath = (char *) malloc(2000);
                sprintf(fullPath, "%s/%s", dir, dirEnt->d_name);
			    enqueue(que, fullPath);
            }
		}
	}
}




void *dirThreadFunc(void *argv) {
    struct DirThreadArgv *dirArgv = (struct DirThreadArgv *) argv;
    for (int i = 0; i < dirArgv->dirCount; i++) {
        //printf("traverse %s\n", (char *) dirArgv->dirs[i]);
        traverseDir(dirArgv->dirs[i], dirArgv->sufix, dirArgv->fileQue);
    }
    free(dirArgv);

    return NULL;
}


queue_t * conductDirs(queue_t *dirQueue, int threadCount, char *sufix) {
    queue_t *fileQueue = createQue();
    int dirCount = dirQueue->size;
    if (dirCount == 0) {
        return fileQueue;
    }

    if (threadCount > dirCount) {
        threadCount = dirCount;
    }

    int dirPerThread = dirCount / threadCount + (dirCount % threadCount != 0);
    
    //printf("%d %d %d\n", dirCount, threadCount, dirPerThread);
    pthread_t *tids = malloc(threadCount * sizeof(pthread_t));

    for (int i = 0; i < threadCount; i++) {
        struct DirThreadArgv *argv = (struct DirThreadArgv *) malloc(sizeof(struct DirThreadArgv));
        argv->sufix = sufix;
        argv->dirs = (char **) malloc( (dirPerThread + 1) * sizeof(char *));
        argv->fileQue = fileQueue;

        int idx = 0;
        while (idx < dirPerThread && dirQueue->size > 0) {
            argv->dirs[idx] = dequeue(dirQueue);
            idx++;
        }
        argv->dirCount = idx;
        pthread_create(&tids[i], NULL, dirThreadFunc, argv);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(tids[i], NULL);
    }

    return fileQueue;
}




void * fileThreadFunc(void *argv) {
    struct FileThreadArgv *fArgv = (struct FileThreadArgv *) argv;
    for (int i = 0; i < fArgv->fcount; i++) {
        //printf("do %s\n", fArgv->files[i]);
        struct HashTable *ht = initHash(DEFUALT_HASH_SIZE);
        int wordCount = countWords(fArgv->files[i], ht);
        calAppearanceRate(ht, wordCount);

        struct WFDInfo *wfdInfo = (struct WFDInfo *) malloc(sizeof(struct WFDInfo));
        char *filePath = (char *) malloc(sizeof(char) * strlen(fArgv->files[i]));
        strcpy(filePath, fArgv->files[i]);
        wfdInfo->file = filePath;
        wfdInfo->ht = ht;

        enqueue(fArgv->wfdQue, wfdInfo);
    }
    free(fArgv);
    return NULL;
}


void * analysisThreadFunc(void *argv) {
    struct AnalysisThreadArgv *aArgv = (struct AnalysisThreadArgv *) argv;
    for (int i = 0; i < aArgv->len; i++) {
        int idx = aArgv->start + i;
        aArgv->tasks[idx].jsdVal = JSD(aArgv->tasks[idx].wfdA->ht, aArgv->tasks[idx].wfdB->ht);
    }
    return NULL;
}



queue_t * conductFiles(queue_t *fileQueue, int threadCount) {
    queue_t *wfdQueue = createQue();
    int fcount = fileQueue->size;
    if (fcount == 0) {
        return wfdQueue;
    }

    if (threadCount > fcount) {
        threadCount = fcount;
    }

    
    int filePerThread = fcount / threadCount + (fcount % threadCount != 0);
    
    //printf("%d %d %d\n", fcount, threadCount, filePerThread);
    pthread_t *tids = malloc(threadCount * sizeof(pthread_t));

    for (int i = 0; i < threadCount; i++) {
        struct FileThreadArgv *argv = (struct FileThreadArgv *) malloc(sizeof(struct FileThreadArgv));
        argv->files = (char **) malloc( (filePerThread + 1) * sizeof(char *));
        argv->wfdQue = wfdQueue;

        int idx = 0;
        while (idx < filePerThread && fileQueue->size > 0) {
            argv->files[idx] = dequeue(fileQueue);
            idx++;
        }
        argv->fcount = idx;
        pthread_create(&tids[i], NULL, fileThreadFunc, argv);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(tids[i], NULL);
    }

    return wfdQueue;
}



int cmp(const void *a, const void *b) {
    struct AnalysisTask taska = *(struct AnalysisTask *) a;
    struct AnalysisTask taskb = *(struct AnalysisTask *) b;

    char bufa[512];
    memset(bufa, 0, 512);
    strcat(bufa, taska.wfdA->file);
    strcat(bufa, taska.wfdB->file);

    char bufb[512];
    memset(bufb, 0, 512);
    strcat(bufb, taskb.wfdA->file);
    strcat(bufb, taskb.wfdB->file);

    return -strcmp(bufa, bufb);
}


void calAndOutputJSD(struct WFDInfo *wfdAry[], int len, int threadCount) {
    if (len <= 1) {
        return;
    }

    int taskCount = len * (len + 1) / 2;
    if (threadCount > taskCount) {
        threadCount = taskCount;
    }

    struct AnalysisTask *tasks = (struct AnalysisTask*) malloc(sizeof(struct AnalysisTask) * taskCount);
    for (int i = 0, k = 0; i < len; i++) {
        for (int j = i; j < len; j++) {
            struct AnalysisTask tk;
            tk.wfdA = wfdAry[i];
            tk.wfdB = wfdAry[j];
            tk.jsdVal = 0.0;
            tasks[k++] = tk;
        }
    }


    int taskPerThread = taskCount / threadCount + (taskCount % threadCount != 0);

    
    //printf("%d %d %d\n", fcount, threadCount, filePerThread);
    pthread_t *tids = malloc(threadCount * sizeof(pthread_t));

    for (int i = 0; i < threadCount; i++) {
        struct AnalysisThreadArgv *argv = (struct AnalysisThreadArgv *) malloc(sizeof(struct AnalysisThreadArgv));
        argv->tasks = tasks;
        argv->start = i * taskPerThread;
        argv->len = taskPerThread;

        if (i == threadCount - 1) { //the last one
            argv->len = taskCount - argv->start;
        }
        
        pthread_create(&tids[i], NULL, analysisThreadFunc, argv);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(tids[i], NULL);
    }
    
    qsort(tasks, taskCount, sizeof(struct AnalysisTask), cmp);

    for (int i = 0; i < taskCount; i++) {
        //printf("task 1 is %s, task 2 is %s\n", tasks[i].wfdA->file, tasks[i].wfdB->file);
    	if(strcmp(tasks[i].wfdA->file, tasks[i].wfdB->file))
	    {
	        printf("%.4lf %s %s\n", tasks[i].jsdVal, tasks[i].wfdA->file, tasks[i].wfdB->file);
   	    }
    }

}


int main(int argc, char *argv[]) {    
    struct CmdLine cmdLine = parseCmd(argc, argv);
    //showCmd(cmdLine);

    queue_t *files = conductDirs(cmdLine.dirQueue, cmdLine.dirThreadCnt, cmdLine.fileNameSuffix);
    while (cmdLine.fileQueue->size > 0) {
        enqueue(files, dequeue(cmdLine.fileQueue));
    }
    //display(files);


    queue_t *wfds = conductFiles(files, cmdLine.fileThreadCnt);
    //displayWfd(wfds);

    int len = 0;
    struct WFDInfo **wfdAry = (struct WFDInfo **) copyToArray(wfds, &len);
    
    calAndOutputJSD(wfdAry, len, cmdLine.analysisThreadCnt);

    return 0;
}
