#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include<stdlib.h>
#define NUM_THREADS 1

void listdir(const char *name, int indent)
{

	//long tid;
	//tid = (long)threadid;

	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(name)))
		return;

	while ((entry = readdir(dir)) != NULL){
		if (entry->d_type == DT_DIR){
			char path[1024];
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
			printf("%*s[%s]\n", indent, "", entry->d_name);
			listdir(path, indent + 2);
		}else{
			printf("%*s- %s\n", indent, "", entry->d_name);
		}
	}

	//pthread_exit(NULL);
	closedir(dir);
}

/*
int main(void){

	listdir(".", 0);
	return 0;

}

*/





void *PrintHello(void *threadid)
{
	long tid;
	tid = (long)threadid;

	printf("Hello World! It's me, thread #%ld!\n", tid);
	
	listdir(".",0);
	pthread_exit(NULL);
}



int main(int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	
	for(t = 0; t < NUM_THREADS; t++)
	{
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);

		if (rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	pthread_exit(NULL);

	return 0;
}

