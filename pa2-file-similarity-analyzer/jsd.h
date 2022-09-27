#ifndef JSD_H
#define JSD_H

#include "hash.h"

double JSD(struct HashTable *ht1, struct HashTable *ht2);

void calAppearanceRate(struct HashTable *ht, int wordCount);

int countWords(char *file, struct HashTable *ht);

#endif