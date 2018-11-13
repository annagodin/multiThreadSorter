#ifndef MULTITHREADSORTER_THREAD
#define MULTITHREADSORTER_THREAD
 //hey

typedef struct CSVrecord {
  char* sortVal;
  char** data;
  //int numCols;
  struct CSVrecord *next;
} CSVrecord;

typedef struct hNode{
	char* data;
	struct hNode *next;
}hNode;

// typedef struct tData{
// 	int unsigned long
// } tData;



//stuff for forking
void *dirwalk(void * argPtr);
int endsWith (char *str, char *end);
void *fileHandler(void* argPtr);



//in scannerCSVsorter.c
char* stripNewLineChar (char* token,int tokLen);
char* stripFirstChar (char* token, int tokLen);
char* stripLastChar (char* token);
char* trimWhiteSpace(char* token);
void printAllRecords (CSVrecord * frontRec);
void printRecNode(CSVrecord *rec);
void addhNodeToEnd(hNode** head, hNode *node);
void addRecToEnd(CSVrecord** head, CSVrecord *node);
void sort(FILE *file, char* fileName);


//in mergesort.c
char* toLowerCase (char* str);
int is_digit(char*str);
struct CSVrecord* SortedMerge(CSVrecord* a, CSVrecord* b);
int compareFields(char* a, char*b);
void FrontBackSplit(CSVrecord* source, CSVrecord** frontRef, CSVrecord** backRef);
void mergesort(CSVrecord** headRef);



#endif
