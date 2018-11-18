

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include "multiThreadSorter_thread.h"
#define _GNU_SOURCE
/**** compile with gcc -pthread -o sorter sorter.c *****/

//pthread_mutex_t lockDIR;
pthread_mutex_t dirMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t lockGlobals;
pthread_mutex_t lockFile;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
int totalThreads = 0; // Number of threads running volatile int totalThreadCount = 0;  
CSVrecord * masterList = NULL;
int masterHasOut = 0;
int masterHasDir = 0;
char* currDir;
char* outputDir;
char* colToSort;
char* searchDir;

//TRY FREEING EVERYTHING BY DECLARING GLOBALLY
//char* trimmed;

//pthread_t threadID[400];
pthread_t* threadID;
int unsigned long init;



char masterHeaders[28][70] = {"color","director_name","num_critic_for_reviews",
"duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name",
"actor_1_facebook_likes","gross","genres","actor_1_name","movie_title",
"num_voted_users","cast_total_facebook_likes","actor_3_name",
"facenumber_in_poster","plot_keywords","movie_imdb_link","num_user_for_reviews",
"language","country","content_rating","budget","title_year","actor_2_facebook_likes",
"imdb_score","aspect_ratio","movie_facebook_likes"};


int isSubstr(char *inpText, char *pattern) {
    int inplen = strlen(inpText);
    while (inpText != NULL) {

        char *remTxt = inpText;
        char *remPat = pattern;

        if (strlen(remTxt) < strlen(remPat)) {
    
            return -1;
        }

        while (*remTxt++ == *remPat++) {
       
            if (*remPat == '\0') {
           
                return inplen - strlen(inpText+1);
            }
            if (remTxt == NULL) {
                return -1;
            }
        }
        remPat = pattern;

        inpText++;
    }
    //printf("\n\n\n\n\t\t%s is a substring of %s\n",pattern,inpText);
    return 0;
}
void freeLL(CSVrecord *frontRec){
	CSVrecord *curr = NULL;
	while ((curr=frontRec)!=NULL){
		frontRec=frontRec->next;
		free(curr);
	}
}
//converts a string to an integer
int stringToInt (char* str){
	
	int dec = 0, i, len;
	len = strlen(str);
	for(i=0; i<len; i++){
		dec = dec * 10 + ( str[i] - '0' );
	}
	return dec;
}
//strips new line character off a string
char* stripNewLineChar (char* token,int tokLen){
	token[tokLen-1]='\0';
	return token;	
}
//strips last character
char* stripLastChar (char* token){
	char* replace = (char*)malloc((strlen(token))*sizeof(char));
	int i;	
	for (i=0; i<(strlen(token)-1); i++){
		replace[i]=token[i];
	}	
	//free(token);
	return replace;	
}
//strips the first character off a string
char* stripFirstChar (char* token, int tokLen){
	char* replace = (char*)malloc((tokLen-1)*sizeof(char));
	int i;
	int j=0;
	for (i=1; i<tokLen; i++){
		replace[j]=token[i];
		j++;
	}
	//free(token);
	return replace;
}
//searches string for a quote
int searchForQuote (char* token){
	int len = strlen(token);
	int i;
	for (i=0; i<len; i++){
		if (token[i]=='"'){
			return 1;
		}
	}
	return 0;
}
//trims white spaces
char* trimWhiteSpace(char* token){
	int index, i;
	index=0;
	i=0;
	char* trimmed = malloc(sizeof(char)*strlen(token)+1);
	//trim leading
	while (token[index] == ' '){
		index++;
	}
	for(i=0;i<strlen(token)-index;i++){
		trimmed[i]=token[index+i];
	}	
	//trim trailing
	index=strlen(trimmed)-1;
	while(trimmed[index] == ' '){
		index--;
	}	
	trimmed[index+1]='\0';	
    //free(token);
    return trimmed;
}
//returns name of header column at specified position
char* getColName(hNode*head, int pos){
	
    int f = 0;
	hNode *ptr = head;
	while (ptr!=NULL){
		if(f==pos){
			return ptr->data;
		}
		ptr=ptr->next;
		f++;

	}	
}
int getMasterIndex(char* headerAtIndex){
	int i;
	for(i=0;i<28;i++){
		//printf("masterHeaders[%d]: %s\n",i, masterHeaders[i]);
		if(strcmp(masterHeaders[i],headerAtIndex)==0){ //found master location
			return i;
		}
	}
	return -1;
}
//adds record node to end;
void addRecToEnd(CSVrecord** head, CSVrecord *node){
	CSVrecord *last = *head;
	
    if (*head == NULL){
       *head = node;
       return;
    }
    while (last->next != NULL)
        last = last->next;
 
    last->next = node;
    return;
}
//helper function for adding header node to end of LL
void addhNodeToEnd(hNode** head, hNode *node){
	hNode *last = *head;
	
    if (*head == NULL){
       *head = node;
       return;
    }
    while (last->next != NULL)
        last = last->next;
 
    last->next = node;
    return;
}
//prints one Record Node
void printRecNode(CSVrecord *rec){
	printf("Record contents:\n");
	int i;
	printf("sortVal:\t%s\n",rec->sortVal);
	//printf("numCols:\t%d\n",rec->numCols);
	printf("data values:\t");
	for(i=0;i<28; i++){
			printf("'%s'\t",rec->data[i]);
	}
	printf("\n");
}
//prints entire linked list of recnodes
void printAllRecords(CSVrecord *frontRec){
	CSVrecord *ptr = frontRec;
	while (ptr!=NULL){
		printRecNode(ptr);
		printf("\n\n");
		ptr=ptr->next;
	}
	printf("\n");
}
//prints in csv format
void printCSV (CSVrecord *frontRec){
	CSVrecord *ptr = frontRec;
	//ptr=ptr->next;
	while(ptr!=NULL){
		int i;
		for(i=0;i<28; i++){		
			if(ptr->data[i]==NULL){
				printf("");
			}else {
				printf("%s",ptr->data[i]);
			}
			
			if(i<27){
				printf(",");
			}
		}
		printf("\n");
		ptr=ptr->next;
	}
}
//writes CSV to a file
void writeCSV (CSVrecord *frontRec, FILE *sorted){
	CSVrecord *ptr = frontRec;
	//ptr=ptr->next;
	while(ptr!=NULL){
		int i;
		for(i=0;i<28; i++){		
			if(ptr->data[i]==NULL){
				fprintf(sorted,"");
			}else {
				fprintf(sorted,"%s",ptr->data[i]);
			}
			
			if(i<27){
				fprintf(sorted,",");
			}
		}
		fprintf(sorted,"\n");
		ptr=ptr->next;
	}
}
//checks if a file ends with a certain string
// (in our case we need to check if it ends with a .csv
int endsWith (char *str, char *end) {
    size_t slen = strlen (str);
    size_t elen = strlen (end);
    if (slen < elen)
        return 0;
    return (strcmp (&(str[slen-elen]), end) == 0);
}
//sort function that takes in a file, col to sort, filename, and outputDir, writes to a new file
void sort(FILE *file, char* fileName){
	
	printf("COL TO SORT FUCKERS: %s\n", colToSort);
	int sortPos=-1;
	char* str;
	str = (char*)malloc(sizeof(char)*1201); //string buffer, tiff changed 1200 -> 1201 for null terminator
	
	char* token;
	token = (char*)malloc(sizeof(char)*200);
	//printf("hey2\n");
	//get headers
	fgets(str, 1200, file);

   	char* rest = (char*)malloc(sizeof(char)*1000);
   	strcpy(rest,str);
   	
	rest[strlen(rest)-1]='\0';
   	//printf("'%s'\n",rest);
   	hNode *headersFront = NULL;
   	int count = 0;

   	//tiff commenting out: pthread_mutex_lock(&mutex2);
   	//tokenizes the headers
   	while ((token = strsep(&rest, ",")) != NULL){
        	//loads headers into array
        	char* data = malloc((strlen(token)+1)*sizeof(char));
        	
        	hNode *node = malloc(sizeof(hNode));
    //     	if (token[strlen(token)-1] == '\n'){
    //     		//printf("stripping newline"); 
				// token=stripNewLineChar(token,strlen(token));
		  //   } 
		    token = trimWhiteSpace(token);
        	strcpy(data,token);
        	node->data=data;
        	addhNodeToEnd(&headersFront, node);
        	//finds col pos to sort by
        	if(strcmp(token,colToSort)==0){
        		sortPos=count;       	
        	}
        	
        	count++;

		//tiff attempting to free node & data
		//free(node);
		//free(data);
       }


		//sets the number of columns
   		int numCols = count;
	//tiff commenting out: pthread_mutex_unlock(&mutex2);
      	
       if(sortPos==-1){
       		fprintf(stderr, "ERROR: Column specified is not a header in the CSV [%s] that is being processed\n",fileName);
       		//printf("ERROR: Column specified is not a header in the CSV [%s] that is being processed\n",fileName);
       		return;
       }

       // printf("GOT HERERERERERE\n");
   //pointer to the front of LL
   CSVrecord * frontRec = NULL;
    
	int i=0;
	
	while(fgets(str,900,file)!=NULL){ //EACH ITERATION IS READING ONE LINE	
		//printf("hey5\n");
		CSVrecord *record = malloc(sizeof(CSVrecord));
		record->next=NULL;
		record->data=malloc(28*sizeof(char*)); 
		
		count=0;

		//printf("STRING: %s\n",str);

		char* parseStr = (char*)malloc((strlen(str)+1)*sizeof(char)+1);
		strcpy(parseStr,str);
		parseStr[strlen(parseStr)-1]='\0'; //strips newline and stuff
			//printf("some testing shit\n");
			int index = 0;
			while ((token = strsep(&parseStr, ",")) != NULL) {
				if (token[strlen(token)-1] == '\n'){ 
					token=stripNewLineChar(token,strlen(token));
				} 			
				//QUOTE CASE if theres a quote at the beginning of a token aka theres a COMMA within the field
		    	if(token[0]=='"'){
		    		//first token in quote
		    		char* append = (char*)malloc((strlen(parseStr)+1)*sizeof(char));	
		    		strcpy(append, token);		
	    			if (token[strlen(token)-1] == '\n'){ 
						token=stripNewLineChar(token,strlen(token));
					}  	//end first token in quote			
					
					//following tokens in quote;
					int counting=1;					
		    		do{		    			 				    			
		    			token = strsep(&parseStr, ",");	
		    			if (token==NULL){
		    				break;
		    			} 
		    			append[strlen(append)]=',';		    			
		    			if (token[strlen(token)-1] == '\n'){ 
							token=stripNewLineChar(token,strlen(token));
						}								 
						counting++;   
						strcat(append,token);				    		
		    		} while (searchForQuote(token)==0); 
		   			
		   			token = append;
				free(append);	//tiff testing the freeing of append		   				   				   					   					   			
		    	} //END QUOTE CASE
		    	
		    	//empty field
		    	if (strcmp(token,"")==0){		    		
		    		token = NULL;
		    	}		    					
				
				//*****TOKEN LOADED INTO A STRUCT
				if(index==sortPos){
					
					if (token==NULL){
						record->sortVal=NULL;
									
					} else{
						record->sortVal=malloc((strlen(token)+1)*sizeof(char));
						strcpy(record->sortVal,token);
						
					}
					
					//TIFF TRYING TO FREE 
					free(record->sortVal);
				}


				//TESTING ASST2 HERE		

				char* headerAtIndexTemp = getColName(headersFront, index);
				char* headerAtIndex = (char*)malloc(strlen(headerAtIndexTemp)*sizeof(char)+1);
				strcpy(headerAtIndex,headerAtIndexTemp);
				//printf("headerAtIndex %d:\t'%s'\n", index, headerAtIndex);

				int masterIndex = getMasterIndex(headerAtIndex);
				if (masterIndex==-1){
					fprintf(stderr, "the column at position [%s] is NOT in the master headers list, cannot sort file\n", index);
					return;
				}
				//printf("token:\t'%s'\n", token);
				//printf("master index:\t%d\n\n", masterIndex);		

				if(token!=NULL){
					//print header at specified index
					
					record->data[masterIndex] = malloc((strlen(token)+1)*sizeof(char));
					strcpy(record->data[masterIndex], token);
					
				} else {
					record->data[masterIndex]=NULL;
					
				}				
				index++;		

			//TIFF ATTEMPTING TO ONCE AGAIN FREE SHIT 
			//free(headerAtIndex);
			//free(record->data);	//might be a problem, lets see
		  	 } //END LINE (RECORD)
		
			//record->numCols=numCols;

			if(index!=numCols){
				fprintf(stderr,"ERROR: [%s] invalid CSV!\n",fileName);
				return;
			}

			//printRecNode(record);

			//ADD RECORD TO LL HERE	
			//tiff commenting out: pthread_mutex_lock(&mutex2);	
			addRecToEnd(&frontRec,record);
			//tiff commenting out: pthread_mutex_unlock(&mutex2);			
			//HERE THE RECORD SHOULD BE COMPLETE
					
		i++;
	} //END FILE
	
	 

	//----------------------------testing printing all records----------------------------	

	// int k;
	// for(k=0;k<28;k++){
		
	// 	if(k==27){
	// 		printf("%s\n",masterHeaders[k]);
	// 	}
	// 	else
	// 		printf("%s,",masterHeaders[k]);
	// }

	//printCSV(frontRec);

	//printf("\n\n\n");	
	//------------------------------------------testing----------------------------------

	//sorts the damn LL
	mergesort(&frontRec);

	//ADDS TO MASTER LIST, MUST BE LOCKED
	pthread_mutex_lock(&mutex1);
	masterList =  SortedMerge(masterList, frontRec);
	printf("sorted the file: [%s]\n", fileName);
	// printAllRecords(masterList);
	pthread_mutex_unlock(&mutex1);


	//testing printing
	//printAllRecords(masterList);

	//printf("-------------------------\nheyyyyy\n-------------------------\n");
	//return;
	

	free(rest);
	free(str);
	 free(token);
	//free(trimmedFileName);
	//fclose(sorted);
	fclose(file);
	// freeLL(frontRec);
	 free(frontRec);	
}

void writeToFile(){
	char* dir;
	if(masterHasOut==1){
		dir = (char*)malloc(sizeof(char)*(strlen(outputDir)+2));
		strcpy(dir,outputDir);
		//get the path for the output directory
		printf("HasOut: output dir is: [%s]\n", outputDir);
	} else {
		//output to cwd
		dir = (char*)malloc(sizeof(char)*(strlen(currDir)+2));
		strcpy(dir,currDir);
		printf("NoOut: output dir is [%s]\n", currDir);
	}
 
	//length of the name of the sorted file
	int lengthSorted = strlen(dir)+strlen(colToSort)+23;
	char sortedFileName[lengthSorted];
	char* extension = ".csv";
	
	snprintf(sortedFileName, lengthSorted, "%sAllFiles-sorted-%s%s", dir, colToSort, extension);
	
	printf("sorted file name:\t%s\n",sortedFileName);
	
	//creates new file with the sorted file name
	FILE *sorted;
	sorted=fopen(sortedFileName, "w");

	if (sorted == NULL){
		fprintf(stderr,"path attempted: [%s]", sortedFileName);
    	fprintf(stderr,"ffopen failed, errno = %d\n", errno);
	}

   	int c=0;
   	//prints headers
   	for (c=0; c<28; c++){
   		if (c==27)
   			fprintf(sorted, "%s\n", masterHeaders[c]);
   		else
   			fprintf(sorted, "%s,", masterHeaders[c]);
   	}

	writeCSV(masterList,sorted);

	//tiff trying to free shit again
	free(dir);
}

//------THREADING STUFF AHEAD--------------------------------------


void *fileHandler(void* argPtr) { 
 	// printf("ThreadID: %lu\n",pthread_self());
	//pthread_mutex_lock(&mutex2);	//comment out later
	char* fileName = (char*)argPtr;
	//struct dirent *entry = (struct dirent*)argPtr;
	printf("FILE: [%s]\n", fileName);
	//pthread_mutex_unlock(&mutex2);	//tiffs edit
	
	// check for csv
	if (!endsWith(fileName, ".csv")){
		fprintf(stderr, "ERROR: [%s] is not a .csv\n", fileName);
		//return;
		pthread_exit(NULL);
	}

	
    FILE *file = fopen(fileName, "r");
    if (file==0){
        fprintf(stderr,"ERROR!!!: %s on file [%s]\n", strerror(errno), fileName);
        //return;
        pthread_exit(NULL);
    }

   	//printf("FILE: [%s]\n", fileName);
   	
    //printf("\t\twill sort the file: [%s] on column [%s] in []\n", entry->d_name, colToSort);
    printf("\t\twill sort the file: [%s] on column [%s]\n", fileName, colToSort);
    
    
    sort(file, fileName);
    
    //fclose(file);
    pthread_exit(NULL);
    //fclose(file);
 
}

//TIFF: TEST DIRWALK FUNCTION
/*
void *dirwalk(void *argPtr){

	char* dir = (char *)argPtr;
	DIR *dp;

	if(!(dp = opendir( 


}

*/






//recursively travserses a directory and prints subdirectories
void *dirwalk(void * argPtr){
   	//printf("YEEEEEEOOOEOEOEOE\n");
    //printf("----------------------\nThreadID: %lu\n",pthread_self());
    char* dir = (char*)argPtr;
    //printf("diririririr:\t%s\n",dir);
    DIR *dp;

	//threadID = malloc((totalThreads + 400) * sizeof(pthread_t));	//tiff test

    printf("this DIR:\t[%s]\n",dir);
    struct dirent *entry;
    struct stat statbuf;
    
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"Error!: cannot open directory: [%s]\n",dir);
        if(totalThreads==0)
        	return;
        else
       		pthread_exit(NULL);
        //exit(EXIT_FAILURE);
    }

   

    chdir(dir);
    //char* current;
    //char* newPath;
    while((entry = readdir(dp)) != NULL){
    	
        lstat(entry->d_name,&statbuf);
        if(entry->d_type==DT_DIR){
        //if(S_ISDIR(statbuf.st_mode)) { //ITS A DIRECTORY, SPAWN A THREAD
        	 
        	 //d_type
        	 //DT_REG
        	 //DT_DIR

        	//pthread_t threadID; 
            //Found a directory , but ignore . and .. 
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0 || strcmp(".git",entry->d_name) == 0)
                continue;

            printf("[%s] is a directory\n\n",entry->d_name);

            // pthread_mutex_lock(&dataMutex); 
            // totalThreads++;
            // pthread_mutex_unlock(&dataMutex);
           	

    		//char currDir[500];
    		//char newPath[500];
    		//tiffs update
    		//char currDir[700];
		//char newPath[700];
		
		//char* currDir = (char*)malloc(strlen(dir)*sizeof(char)+1);
  		//	strcpy(currDir,dir);
            //char* newPath=(char*)malloc((strlen(currDir)+strlen(entry->d_name)+3)*sizeof(char));
		  //  strcpy(newPath, currDir); //copy directory over

    		
    		pthread_mutex_lock(&mutex1);
    		//char* current = (char*)malloc(strlen(dir)*sizeof(char)+2);
		char* current = (char*)malloc(strlen(dir)*sizeof(char)+2);	//tiffs edit
  			strcpy(current,dir);
            	
             char* newPath=(char*)malloc((strlen(current)+strlen(entry->d_name)+3)*sizeof(char)); 
	    //^commented out to handle outside while loop
		    strcpy(newPath, current); //copy directory over
			
			// strcat(newPath, "/"); 
			strcat(newPath,entry->d_name); //update the directory- our new working directory updated
			//printf("DIRDIR newPath: %s\n",newPath);

			strcat(newPath, "/"); 
		 pthread_mutex_unlock(&mutex1);
			//return;
            // pthread_create(&threadID[totalThreads], NULL,(void*)&dirwalk, (void*)entry->d_name);
            
			//printf("newpath!!!! [%s]\n",newPath);
	    //tiff adding mutex lock for thread creation
	    pthread_mutex_lock(&mutex1);	
            pthread_create(&threadID[totalThreads], NULL, (void*)&dirwalk, (void*)newPath);
            pthread_mutex_unlock(&mutex1);
	    pthread_mutex_lock(&dirMutex);
            totalThreads++;
            //pthread_mutex_lock(&mutex2); //tiff: test
	    //pthread_mutex_unlock(&mutex2);
            printf("totalThreads:%d\n",totalThreads);
            pthread_mutex_unlock(&dirMutex);

            //recurse here
            /*funtion is called recursively at a new indent level */
            //dirwalk((void*)entry->d_name);
            //dirwalk((void*)newPath);
            
            //free(currDir);
            //TIFF JUST TESTING FREEING AFTER EACH ITERATION
            	//free(newPath);
	    	//free(current);
         
        }
        //else if(S_ISREG(statbuf.st_mode)){ //ITS A FILE, SPAWN A THREAD
        else if(entry->d_type==DT_REG){
            printf("\t[%s] is a file\n",entry->d_name);
            //printf("path: %s\n",dir);
            //pthread_t threadFile;
	      
	     char* newPath=(char*)malloc((strlen(dir)+strlen(entry->d_name)+3)*sizeof(char)*5);
             //^tiff commented out to handle outside while loop
		//printf("just created space for newPath\n");
		

		//pthread_mutex_unlock(&mutex1);  
		//char* newPath=(char*)malloc((strlen(dir)+strlen(entry->d_name)+3)*sizeof(char));
		 	 strcpy(newPath, dir); //copy directory over
			printf("just copied directory over\n");	
			 // strcat(newPath, "/"); 
			 strcat(newPath,entry->d_name); //update the directory- our new working directory updated
			 //printf("newPath FILE NAME: %s\n",newPath);
          
			printf("just updated the directory\n"); 
			//printf("newPath FILE NAME: %s\n",newPath);	
           	//tiff adding mutex lock for thread creation
           	pthread_mutex_lock(&mutex1);
		pthread_create(&threadID[totalThreads], NULL,(void*)&fileHandler, (void*)newPath);
        	printf("just created a new thread!\n");
		pthread_mutex_unlock(&mutex1);
		pthread_mutex_lock(&fileMutex);
		printf("just locked totalThread global var to prepare for increment op\n");
		totalThreads++;
		printf("totalThreads:%d\n", totalThreads);
       		pthread_mutex_unlock(&fileMutex);
		printf("just unlocked totalThread incrementation\n");
        
		//just a test -- tiff, trying to free memory for next iteration
			//free(newPath);
		
	}

    }
		//tiffs attempt at freeing current and new path after declaring
		//malloc outside of while loop
   			//free(current);
			//free(newPath);
   		//printf("*init: %lu\n",init);
   		// printf("**hey sup: totalThreads:%d\n",totalThreads);
   		// printf("***hey sup current thread:%lu\n",pthread_self());
   		
		
		
   		// if(totalThreads>1)
    	// pthread_exit(NULL);

   		// if(pthread_self()==init){
   		// 	printf("WE IN THE INIT BITCH\n\n");
   		// 	// return;
   		// } else {
   		// 	pthread_exit(NULL);
   		// }
   			

   		chdir("..");
   		closedir(dp);
    	// printf("HELLO ANYBODY THERE\n");
    	//tiff test
    	//pthread_mutex_destroy(&mutex1);
	//pthread_mutex_destroy(&mutex2);


    	pthread_exit(NULL);
   		
}

//------END Threading STUFF-------------------------------------

int main(int argc, char *argv[] ){ //-----------------------MAIN---------
	
	int hasDir=0;
	int hasOut=0;
	int hasCol=0;
	//char* searchDir;
	
	
	//pthread_mutex_init(&mutex1, NULL);


	if(argc<3){
		fprintf(stderr,"Error, not enough arguments!\n");
		exit(EXIT_FAILURE);
	}
	if(argc>7){
		fprintf(stderr,"Error: Too many parameters\n");
		fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");			
		exit(EXIT_FAILURE);
	}
	if (argc==4||argc==6){
		fprintf(stderr,"Error: Incorrent arguments\n");
		fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
		exit(EXIT_FAILURE);
	}
	if (argc>=3){ //at least 1 flag
		if(strcmp(argv[1], "-c") == 0){
			colToSort = (char*)malloc(strlen(argv[2])*sizeof(char)+1);
			strcpy(colToSort,argv[2]);
			hasCol=1;
		} else if (strcmp(argv[1], "-d") == 0){
			searchDir = (char*)malloc(strlen(argv[2])*sizeof(char)+1);
			strcpy(searchDir,argv[2]);
			hasDir=1;
		} else if (strcmp(argv[1], "-o") == 0){
			outputDir = (char*)malloc(strlen(argv[2])*sizeof(char)+1);
			strcpy(outputDir,argv[2]);
			hasOut=1;
		} else {
			fprintf(stderr,"Error: Incorrect parameters\n");
			fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}
	}
	if (argc>=5){ //at least 2 flags
		if(strcmp(argv[3], "-c") == 0){
			colToSort = (char*)malloc(strlen(argv[4])*sizeof(char)+1);
			strcpy(colToSort,argv[4]);
			if (hasCol==0)
				hasCol=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[3], "-d") == 0){
			searchDir = (char*)malloc(strlen(argv[4])*sizeof(char)+1);
			strcpy(searchDir,argv[4]);
			if (hasDir==0)
				hasDir=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[3], "-o") == 0){
			outputDir = (char*)malloc((strlen(argv[4])+1)*sizeof(char));
			strcpy(outputDir,argv[4]);
			if (hasOut==0)
				hasOut=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else {
			fprintf(stderr,"Error: Incorrect parameters\n");
			fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}
	}
	if (argc==7){ //all 3 flags
		if(strcmp(argv[5], "-c") == 0){
			colToSort = (char*)malloc(strlen(argv[6])*sizeof(char)+1);
			strcpy(colToSort,argv[6]);
			if (hasCol==0)
				hasCol=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[5], "-d") == 0){
			searchDir = (char*)malloc(strlen(argv[6])*sizeof(char)+1);
			strcpy(searchDir,argv[6]);
			if (hasDir==0)
				hasDir=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[5], "-o") == 0){
			outputDir = (char*)malloc(strlen(argv[6])*sizeof(char)+1);
			strcpy(outputDir,argv[6]);
			if (hasOut==0)
				hasOut=1;
			else {
				fprintf(stderr,"Error: Incorrent arguments\n");
				fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
				exit(EXIT_FAILURE);
			}
		} else {
			fprintf(stderr,"Error: Incorrect parameters\n");
			fprintf(stderr,"Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}
	}

	if(hasCol==0){
		fprintf(stderr,"Error: must have -c flag. Other flags are optional");
		exit(EXIT_FAILURE);
	}
	
	printf("colToSort:\t%s\n",colToSort);
	printf("hasC %d, hasD %d, hasO %d\n",hasCol,hasDir,hasOut);
	if(hasDir){
		masterHasDir=1;
		DIR *dp;
		if((dp = opendir(searchDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",searchDir);
	        exit(EXIT_FAILURE);
   		}
   		printf("searchDir:\t%s\n",searchDir);	
	}

	if(hasOut){
		masterHasOut=1;
		DIR *dp;
		if((dp = opendir(outputDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",outputDir);
	        exit(EXIT_FAILURE);
   		}
   		//if the specified output directory does not have a slash at the end - add one
		if (strcmp(outputDir,"")!=0&&outputDir[strlen(outputDir)-1]!='/'){
			char appendSlash[strlen(outputDir)+2];
			strcpy(appendSlash,outputDir);
			strcat(appendSlash,"/");
			outputDir=malloc((strlen(outputDir)+2)*sizeof(char));
			strcpy(outputDir,appendSlash);
		}
   		printf("outputDir:\t%s\n",outputDir);	
	}

//	return 0;
	
// //---------------------testing sort function----------------------------------	
	// FILE *file = fopen("smalldata.csv", "r");
	// if (file==0){
	// 	printf("ERROR: %s\n", strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }	

	// int i;
	// for(i=0;i<28;i++){
	// 	printf("masterHeaders[%d]: '%s'\n",i, masterHeaders[i]);
	// }

	//  sort(file,"smalldata.csv");
	//  //return 0;
//---------------------end testing sort function--------------------------------------

	
	char cwd[400];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    	perror("getcwd() error");
  	currDir = (char*)malloc(strlen(cwd)*sizeof(char)+1);
  	strcpy(currDir,cwd);
  	strcat(currDir,"/");
	

	//testing stuffs
	// writeToFile();

	
	char *outputFull;
	if(hasOut){
		if(outputDir[0]=='/'){
		printf("absolute file name\n");
			outputFull = (char*)malloc(strlen(outputDir)+2);
			strcpy(outputFull,outputDir);
		}
		else {
		printf("relative file name\n");
			outputFull = (char*)malloc(strlen(currDir)+3+strlen(outputDir));
			
			strcpy(outputFull,currDir);
			//strcat(outputFull,"/");
			strcat(outputFull, outputDir);
			
			printf("\toutput full %s\n\n",outputFull);
			outputDir = (char*)malloc(strlen(currDir)+3+strlen(outputDir));
			strcpy(outputDir,outputFull);
			//MIGHT BE A SOURCE OF MEM LEAK^
			free(outputFull);	//tiff attempt to free		
		}
	} else { //no output directory specified, use the current directory
		outputDir = (char*)malloc((strlen(currDir)+2)*sizeof(char));
		strcpy(outputDir,currDir);
	}	


	char *searchFull;
	if(hasDir){
		if(searchDir[strlen(searchDir)-1]!='/')
			strcat(searchDir,"/");

		if(searchDir[0]=='/'){
		//printf("absolute file name\n");
			searchFull = (char*)malloc(strlen(searchDir)+2);
			strcpy(searchFull,searchDir);
			free(searchFull);	//tiffs attempt at freeing malloc
		}
		else {
			//printf("relative file name\n");
			searchFull = (char*)malloc(strlen(currDir)+3+strlen(searchDir));
			strcpy(searchFull,currDir);
			//strcat(searchFull,"/");
			strcat(searchFull, searchDir);
			searchDir = (char*)malloc(strlen(searchFull)*sizeof(char)+1);
			strcpy(searchDir,searchFull);
			//printf("\t\t\t\toutput full %s\n\n",searchFull);
			free(searchFull);	//tiffs attempt at freeing malloc
		}
	}

	printf("THEEEE OUTPUT DIR IS : %s\n", outputDir);

	printf("THEEEE SEARCH DIR IS : %s\n", searchDir);
	
	printf("THEEEE CURR DIR IS : %s\n",currDir);
	
	init = pthread_self();
	fprintf(stdout, "\nInitial TID: %lu\n", init);
	printf("Initial Thread Count:%d\n",totalThreads);
	/*test---------------------*/
	//return 0;
	/*test---------------------*/

	//pthread_t *threadID;
	//threadID = (pthread_t *)(malloc(sizeof(*threadID)+400)); //tiff test 
 	
	threadID = (pthread_t *)malloc(sizeof(pthread_t) * 400);

	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL);	

	// char search[strlen(searchDir)+1];
	// char 

	if(pthread_mutex_init(&dirMutex, NULL) != 0){
        printf("Error on lock1.\n");
    }
    if(pthread_mutex_init(&fileMutex, NULL) != 0){
        printf("Error on lock2.\n");
    }
    if(pthread_mutex_init(&mutex1, NULL) != 0){
        printf("Error on lock3.\n");
    }
    if(pthread_mutex_init(&mutex2, NULL) != 0){
        printf("Error on lock4.\n");
    }

	if(hasDir == 1 && hasOut == 0) { //-d 
		printf("HEYEYEYEY\n");
		pthread_mutex_lock(&mutex1); //tiffs test
		pthread_create(&threadID[totalThreads], NULL, (void*)&dirwalk, (void*)searchDir);
		totalThreads++;
		pthread_mutex_unlock(&mutex1);
		//dirwalk((void*)searchDir);
	} else if(hasDir  == 1 && hasOut == 1)	{ //-d and -o
		pthread_mutex_lock(&mutex1);
		pthread_create(&threadID[totalThreads], NULL, (void*)dirwalk, (void*)searchDir);
		totalThreads++;
		pthread_mutex_unlock(&mutex1);
		//dirwalk((void*)searchDir);
	} else if(hasDir  == 0 && hasOut == 1)	{ //-o
		pthread_mutex_lock(&mutex1);
		pthread_create(&threadID[totalThreads], NULL, (void*)dirwalk, (void*)currDir);
		totalThreads++;
		pthread_mutex_unlock(&mutex1);
		//dirwalk((void*)cwd);
	} else { //neither
		pthread_create(&threadID[totalThreads], NULL, (void*)dirwalk, (void*)currDir);
		//dirwalk((void*)cwd);
	}
	// fclose(pidRec);

	
	//return;
	


	int i;
	
	for (i =0; i < totalThreads; i++) {
		pthread_join(threadID[i], NULL);
		printf("\t\t\t\t\tnum THREADERRRRS: %d\n", totalThreads); 
	}	

	printf("d0ne?\n");

	//tiff test
	//pthread_mutex_destroy(&mutex1);
	//pthread_mutex_destroy(&mutex2);

	
	//printf("Total number of threads spawned: %d\n", totalThreads);


	fprintf(stdout,"TIDS of all spawned threads: ");
	
	for (i =0; i < totalThreads; i++) {
		printf("%lu,",threadID[i]); 
	}
	fprintf(stdout,"\nTotal number of threads: %d\n", totalThreads);

   		//}
	
	//free threadID
	/*
	for(i = 0; i < totalThreads; i++){
		free(threadID[i]);
	}
	*/
	
	pthread_exit(threadID);
	//free(threadID);


	// printAllRecords(masterList);
	writeToFile();
	
	free(currDir);
	
	//Tiff edit: attempting to free left over malloc'd stuff
	if(hasDir){
		//free(searchFull);
		free(searchDir);
	}	
	if(hasOut){
		//free(outputFull);
		free(outputDir);
	}

	free(colToSort);
		//free(trimmed);	//try to free trimmed var in trimWhiteSpace()	
	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);


	// pthread_exit(threadID);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&dirMutex);
	
	free(threadID);	//tiff - moved free stmt to end	
	free(masterList);	//never freed whole thing? lets try
	
	return 0;
} //-----------------------------------ENDMAIN-------------------
