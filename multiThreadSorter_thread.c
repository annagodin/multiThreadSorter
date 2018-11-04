

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "multiThreadSorter_thread.h"
#include <errno.h>
#include <limits.h>

//-c blah -d boo -o haii


int numProcesses=0;
//pidNode *pidHead = NULL;


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
	/*char* replace = (char*)malloc((tokLen-1)*sizeof(char));
	int i;
	for (i=0; i<tokLen-1; i++){
		replace[i]=token[i];
	}

	return replace;*/
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
	printf("numCols:\t%d\n",rec->numCols);
	printf("data values:\t");
	for(i=0;i<rec->numCols; i++){
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
		for(i=0;i<ptr->numCols; i++){		
			if(ptr->data[i]==NULL){
				printf("");
			}else {
				printf("%s",ptr->data[i]);
			}
			
			if(i<ptr->numCols-1){
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
		for(i=0;i<ptr->numCols; i++){		
			if(ptr->data[i]==NULL){
				fprintf(sorted,"");
			}else {
				fprintf(sorted,"%s",ptr->data[i]);
			}
			
			if(i<ptr->numCols-1){
				fprintf(sorted,",");
			}
		}
		fprintf(sorted,"\n");
		ptr=ptr->next;
	}

}

//sort function that takes in a file, col to sort, filename, and outputDir
//writes to a new file
void sort(FILE *file, char *colToSort, char* fileName, char *outputDir){
	// printf("coltosort: [%s]\n",colToSort);
	//printf("output dirrrrrrrr [%s]\n",outputDir);

	//if the specified output directory does not have a slash at the end - add one
	if (strcmp(outputDir,"")!=0&&outputDir[strlen(outputDir)-1]!='/'){
		char appendSlash[strlen(outputDir)+2];
		strcpy(appendSlash,outputDir);
		strcat(appendSlash,"/");
		outputDir=malloc((strlen(outputDir)+2)*sizeof(char));
		strcpy(outputDir,appendSlash);
	}
	

	//printf("\t\t\t\t\tfile is [%s]\n",fileName);


	int sortPos=-1;
	char* str;
	str = (char*)malloc(sizeof(char)*1200); //string buffer
	
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
   //	printf("hey3\n");

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
       }



 		//sets the number of columns
   		int numCols = count;

 
//------------------------test headers
  //      int f =0;
		// hNode *ptr1 = headersFront;
  //  		while (ptr1!=NULL){
	 //   		printf("'%s'",ptr1->data);
	 //   		ptr1=ptr1->next;
	 //   		if(f<numCols-1){
	 //   			printf(",");
	 //   		}
	 //   		f++;
  //  		}
  //  		printf("\n\n\n");	
//---------------------test headers


      	//printf("\tjwejejejjet\t32329238\theyyy22yyyy11111\n");
		//printf("\t\t\t\t----------[%s]\n",fileName);
       if(sortPos==-1){
       		fprintf(stderr, "ERROR: Column specified is not a header in the CSV [%s] that is being processed\n",fileName);
       		//printf("ERROR: Column specified is not a header in the CSV [%s] that is being processed\n",fileName);
       		return;
       }



  
   
   


   //printf("cp1\n");

   //pointer to the front of LL
   CSVrecord * frontRec = NULL;
    
	int i=0;
	
	while(fgets(str,900,file)!=NULL){ //EACH ITERATION IS READING ONE LINE	
		//printf("hey5\n");
		CSVrecord *record = malloc(sizeof(CSVrecord));
		record->next=NULL;
		record->data=malloc(numCols*sizeof(char*)); 
		
		count=0;

		//printf("STRING: %s\n",str);

		char* parseStr = (char*)malloc((strlen(str)+1)*sizeof(char)+1);
		strcpy(parseStr,str);
		parseStr[strlen(parseStr)-2]='\0'; //strips newline and stuff
			//printf("some testing shit\n");
			int index = 0;
			while ((token = strsep(&parseStr, ",")) != NULL) {
				//printf("hey6\n");
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
		    	} //END QUOTE CASE
		    	
		    	//printf("TOKEN: %s\n",token);
		    	

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
				}
				
				if(token!=NULL){
					
					record->data[index] = malloc((strlen(token)+1)*sizeof(char));
					strcpy(record->data[index], token);
					
				} else {
					record->data[index]=NULL;
					
				}				
				index++;		

				
				

				

		  	 } //END LINE (RECORD)
			


			record->numCols=numCols;

			if(index!=numCols){
				fprintf(stderr,"ERROR: [%s] invalid CSV!\n",fileName);
				return;
			}		
			//printRecNode(record);

			//ADD RECORD TO LL HERE		
			addRecToEnd(&frontRec,record);			
			//HERE THE RECORD SHOULD BE COMPLETE
					
		i++;
	} //END FILE
	




//-------------------------testing printing all records
	// int f =0;
	// hNode *ptr1 = headersFront;
 //   	while (ptr1!=NULL){
 //   		printf("%s",ptr1->data);
 //   		ptr1=ptr1->next;
 //   		if(f<numCols-1){
 //   			printf(",");
 //   		}
 //   		f++;
 //   	}
 //   	printf("\n");	

	// printAllRecords(frontRec);
	// printf("\n\n\n");	
//-------------------------testing

	


	//sorts the damn LL
	mergesort(&frontRec);


	//printf("\tfilename: %s\n",fileName);


	//length of the name of the sorted file
	int lengthSorted = strlen(outputDir)+strlen(fileName)+strlen(colToSort)+11;
	char sortedFileName[lengthSorted];
	
	//trim the .csv off the file
	char* trimmedFileName=malloc((strlen(fileName)+1)*sizeof(char));
	strcpy(trimmedFileName,fileName);
	trimmedFileName[strlen(fileName)-4]='\0';

	//printf("trimmed file name is %s\n",trimmedFileName);
	char* extension = ".csv";
	
	//if output directory specified, add slash after that dir, if not then no slash
	if(strcmp(outputDir,"")!=0)
		snprintf(sortedFileName, lengthSorted, "%s%s-sorted-%s%s", outputDir, trimmedFileName, colToSort, extension);
	else
		snprintf(sortedFileName, lengthSorted, "%s-sorted-%s%s", trimmedFileName, colToSort, extension);
	
	//printf("sorted file name:\t%s\n",sortedFileName);
	
	//creates new file with the sorted file name
	FILE *sorted;
	sorted=fopen(sortedFileName, "w");
	

	if (sorted == NULL){
		fprintf(stderr,"path attempted: [%s]", sortedFileName);
    	fprintf(stderr,"ffopen failed, errno = %d\n", errno);
	}

	
   	int c=0;
   	//prints headers
   	hNode *ptr = headersFront;
   	while (ptr!=NULL){
   		fprintf(sorted,"%s",ptr->data);
   		ptr=ptr->next;
   		if(c<numCols-1){
   			fprintf(sorted,",");
   		}
   		c++;
   	}
   	fprintf(sorted, "\n");	

	writeCSV(frontRec,sorted);

	//printf("sorted: %d\n",sorted);
	
	
	free(rest);
	free(str);
	free(token);
	free(trimmedFileName);
	//fclose(sorted);
	fclose(file);
	// freeLL(frontRec);
	free(frontRec);
	


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












//FORKING STUFF AHEAD--------------------------------------



//recursively travserses a directory and prints subdirectories
void dirwalk(char *dir,char *out, char *colToSort, FILE *fp){
    //printf("hai\n");
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    int status1, status2;
    int pid1, pid2;
    //printf("\tOUT!!! [%s]\n",out);
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"Error: cannot open directory: %s\n",dir);
        exit(EXIT_FAILURE);
    }

    chdir(dir);

    
    //printf("Starting dir:\t[%s]\n", dir);
	
    while((entry = readdir(dp)) != NULL){
    	//printf("ha3\n");
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) { //ITS A DIRECTORY
            //Found a directory , but ignore . and .. 
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0 || strcmp(".git",entry->d_name) == 0)
                continue;

            //**printf("Found directory\t[%s]\tdir\t[%s]\n",entry->d_name,dir);
           // printf("%s/\n",entry->d_name); //FORK HERE AND RECURSE
            pid1=fork();
            if(pid1==0){ //child
                //recurse here
                /*funtion is called recursively at a new indent level */
                dirwalk(entry->d_name,out,colToSort,fp);
                exit(0);
            } else { //parent
                //fprintf(stdout,"%d,",getpid());
               
                //printf("PID1:\t\t\t%d\n",pid1);
                
                fprintf(fp, "%d\n", pid1);
                fflush(fp); 
               	
               	
                //waitpid(pid1, &status1, WUNTRACED);
              
            }

           //printf("hai3\n");
            
        }
        else if(S_ISREG(statbuf.st_mode)){ //ITS A FILE, FORK TO SORT FILE
            //printf("ha433\n");
            //printf("its a csvvvvvv\n");
            //if (endsWith(entry->d_name, ".csv")){
            	
                //**printf("found a csv\t[%s]\tdir\t[%s]\n",entry->d_name, dir);
                

                if(isSubstr(entry->d_name, "1234proc.txt")!=-1){
                	continue;
                }

                //printf("%s \n",entry->d_name); 
                pid2=fork();
                if(pid2==0){ //child
                    
                    //check for csv
            		if (!endsWith(entry->d_name, ".csv")){
            			fprintf(stderr, "ERROR: [%s] is not a .csv\n", entry->d_name);
            			exit(1);
            		}

					if(isSubstr(entry->d_name, "-sorted-")!=-1){
                		//printf("\t\t\tBRUHHHHH ITS SORTEDDDD\n");
                		fprintf(stderr, "ERROR: [%s] is already sorted\n",entry->d_name);
                		exit(1);
                		//**printf("the csv\t[%s]\tdir\t[%s] has the word 'sorted', dont touch\n",entry->d_name, dir);
                		
                	}
                    FILE *file = fopen(entry->d_name, "r");
                    if (file==0){
                        fprintf(stderr,"ERROR: %s\n", strerror(errno));
                        exit(1);
                    }
                    
                    sort(file, colToSort, entry->d_name, out);
                    exit(0);

                } else { //parent
					//fprintf(stdout,"%d,",getpid());                   
                    //printf("PID2:\t\t\t%d\n",pid2);
                  	//printf("\n");
                    fprintf(fp, "%d\n", pid2);
                    fflush(fp); 
                   	
                   
                    waitpid(pid2, &status2, WUNTRACED);
                   
                 }
            
            //}
             

        }

    }


     	chdir("..");
   		closedir(dp);

   		wait(NULL);
}




//counts the number of processes that this directory search will require
int numProc(char *dir){
    int numP=0;
    DIR *dp;
    char str[80]; 
    char name[80]; 
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"Error: cannot open directory: %s\n",dir);
        exit(EXIT_FAILURE);
    }

    chdir(dir);

    while((entry = readdir(dp)) != NULL){
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) { //ITS A DIRECTORY
            /* Found a directory , but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0 || strcmp(".git",entry->d_name) == 0)
                continue;
            numP++;
            //printf("nump bc found directory, [%s]:\t%d\n",entry->d_name,numP);
            //printf("Found directory\t[%s]\tdir\t[%s]\tnumP:\t[%d]\n",entry->d_name,dir,numP);
           
            /*funtion is called recursively at a new indent level */
            numP += numProc(entry->d_name);
            //printf("nump after recursing %s:\t%d\n",entry->d_name,numP);
        }
        else if(S_ISREG(statbuf.st_mode)){ //ITS A FILE, FORK TO SORT FILE
            //if (endsWith (entry->d_name, ".csv")){
                
                // if(isSubstr(entry->d_name, "-sorted-")!=-1){
                // 	//**printf("the csv\t[%s]\tdir\t[%s] has the word 'sorted', dont touch\n",entry->d_name, dir);
                // 	continue;
                // }

                numP++;
                //printf("found a csv, [%s] in dir [%s] nump:\t%d\n",entry->d_name, dir,numP);
           		//printf("found a csv\t[%s]\tdir\t[%s]\tnumP:\t[%d]\n",entry->d_name, dir,numP);
                

           // }  
        }
    }

        chdir("..");
        closedir(dp);
        return numP;
}







//END FORKING STUFF-------------------------------------













int main(int argc, char *argv[] ){ //-----------------------MAIN---------
	
	int hasDir=0;
	int hasOut=0;
	int hasCol=0;
	char* searchDir;
	char* outputDir;
	char* colToSort;
	// argv[1] //"-c"
	// argv[2] //-c val
	// argv[3] //"-d"
	// argv[4] //-d val
	// argv[5] //"-o"
	// argv[6] //-o val

	//if no -d, only search current directory
	//if no -o, output to current directory
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
	
	if(hasDir){
		DIR *dp;
		if((dp = opendir(searchDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",searchDir);
	        exit(EXIT_FAILURE);
   		}
   		printf("searchDir:\t%s\n",searchDir);	
	}

	if(hasOut){
		DIR *dp;
		if((dp = opendir(outputDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",outputDir);
	        exit(EXIT_FAILURE);
   		}
   		printf("outputDir:\t%s\n",outputDir);	
	}

//	return 0;




	
// //---------------------testing sort	
// 	FILE *file = fopen("movie_metadata.csv", "r");
// 	if (file==0){
// 		printf("ERROR: %s\n", strerror(errno));
// 		exit(EXIT_FAILURE);
// 	}	


// 	 sort(file, colToSort, "movie_metadata.csv", "");
// 	 return 0;
//---------------------end testing sort

	// printf("DONE SORT TEST\n");
	// return 0;
	// char* cwd = (char*)malloc(100*sizeof(char));
	// cwd = getcwd(cwd, sizeof(cwd));
	// printf("this is the current dir: [%s]\n",cwd);
	
	char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    	perror("getcwd() error");
  	char* currDir = (char*)malloc(strlen(cwd)*sizeof(char)+1);
  	strcpy(currDir,cwd);
	
  	int numP;
	if(hasDir==0){
		numP = numProc(currDir);
	} else
		numP=numProc(searchDir);


	//printf("numProc is \t%d\n\n\n",numP);


	char * fname = (char*)malloc(strlen(currDir)+14);
	strcpy(fname, currDir);
	strcat(fname,"/");
	strcat(fname,"1234proc.txt");
	FILE *pidRec;
	pidRec=fopen(fname, "w");
	//fprintf(pidRec, "PIDS:\n");
	//fclose(pidRec);

	//printf("fname is %s\n", fname);
	//printf("currdir %s\n",currDir);
	char *outputFull;
	if(hasOut){
		if(outputDir[0]=='/'){
		//printf("absolute file name\n");
			outputFull = (char*)malloc(strlen(outputDir)+2);
			strcpy(outputFull,outputDir);
		}
		else {
			printf("relative file name\n");
			outputFull = (char*)malloc(strlen(currDir)+3+strlen(outputDir));
			strcpy(outputFull,currDir);
			strcat(outputFull,"/");
			strcat(outputFull, outputDir);
			printf("\t\t\t\toutput full %s\n\n",outputFull);
		}
	}	

	// char* outputFull;
	// realpath(outputDir,outputFull);
	// printf("fullpath: [%s]\n",outputFull);
	

	if(hasDir == 1 && hasOut == 0) { //-d 
		dirwalk(searchDir, "", colToSort, pidRec);
	} else if(hasDir  == 1 && hasOut == 1)	{ //-d and -o
		dirwalk(searchDir, outputFull, colToSort, pidRec);
	} else if(hasDir  == 0 && hasOut == 1)	{ //-o
		dirwalk(cwd, outputFull, colToSort, pidRec);
	} else { //neither 
		dirwalk(cwd, "", colToSort, pidRec);
	}
	fclose(pidRec);


	int init = getpid();
	fprintf(stdout, "\nInitial PID: %d\n", init);

	fprintf(stdout,"PIDS of all child processes: ");
	
	wait();

	FILE *pidPrint;
	pidPrint = fopen(fname, "r");
	char *str =(char*)malloc(100*sizeof(char));

	int counting = 0;

	while(fgets(str,900,pidPrint)!=NULL){
		counting++;
		str=stripNewLineChar(str,strlen(str));
		printf("%s, ",str);
	}

	//fprintf(stdout,"\nTotal number of processes: %d\n",(numP+1));
	fprintf(stdout,"\nTotal number of processes: %d\n",(counting+1));

	fclose(pidPrint);
	remove(fname);

	free(currDir);
	free(fname);
	free(str);
	
	if(hasDir)
		free(searchDir);
	if(hasOut)
		free(outputDir);
	
	free(colToSort);

	return 0;
} //-----------------------------------ENDMAIN-------------------