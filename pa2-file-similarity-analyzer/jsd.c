#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "jsd.h"
#include "hash.h"

int countWords(char *file, struct HashTable *ht) {
    FILE *fp;
    fp = fopen (file, "r");
    int wordCount = 0;
    int retv = 0;
    do {
        char *word = (char *) malloc(80 * sizeof(char));
        memset(word, 0, 0);
        retv = fscanf(fp, "%s", word);
        if (strlen(word) == 0) {
            continue;
        }

        wordCount++;
        struct Entry *entry = get(ht, word); 
        if (entry == NULL) {
            put(ht, word, 1);
        } else {
            put(ht, word, entry->value + 1);
        }
    } while (~retv);

    return wordCount;
}


void calAppearanceRate(struct HashTable *ht, int wordCount) {
    for (int i = 0; i < ht->tableLen; i++) {
        struct Node *temp = ht->lists[i]->next;
        if (temp == NULL) {
            continue;
        }
        while (temp) {
            temp->entry->value /= (double) wordCount;
            temp = temp->next;
        }
    }
}


void calAvgHelper(struct HashTable *ht1, struct HashTable *ht2, struct HashTable *htAvg) {
    for (int i = 0; i < ht1->tableLen; i++) {
        struct Node *temp = ht1->lists[i]->next;
        if (temp == NULL) {
            continue;
        }
        while (temp) {
            struct Entry *entry1 = temp->entry;
            struct Entry *entry2 = get(ht2, entry1->key);
            
            double rate = entry1->value;
            if (entry2 != NULL) {
                rate += entry2->value;
            }

            put(htAvg, entry1->key, rate / 2.0);

            temp = temp->next;
        }
    }
}

struct HashTable * calAvg(struct HashTable *ht1, struct HashTable *ht2) {
    struct HashTable *htAvg = initHash(DEFUALT_HASH_SIZE);
    calAvgHelper(ht1, ht2, htAvg);
    calAvgHelper(ht2, ht1, htAvg);
    return htAvg;
}


double KLD(struct HashTable *ht, struct HashTable *htAvg) {
    double res = 0;
    for (int i = 0; i < ht->tableLen; i++) {
        struct Node *temp = ht->lists[i]->next;
        if (temp == NULL) {
            continue;
        }
        while (temp) {
            double v = temp->entry->value;
            double avgV = get(htAvg, temp->entry->key)->value;
            res += v * (log(v / avgV) / log(2));
            temp = temp->next;
        }
    }
    return res;
}



double JSD(struct HashTable *ht1, struct HashTable *ht2) {
    struct HashTable *htAvg = calAvg(ht1, ht2);
    double f1_favg = KLD(ht1, htAvg);
    double f2_favg = KLD(ht2, htAvg);
    return sqrt(f1_favg / 2.0 + f2_favg / 2.0);
}