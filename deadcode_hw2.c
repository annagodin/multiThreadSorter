//-----------------------------------------------------------------
		if(strcmp(argv[1], "-c") == 0){
			colToSort = (char*)malloc(strlen(argv[2])*sizeof(char)+1);
			strcpy(colToSort,argv[2]);
		} else {
			printf("Error, invalid flag!\n");
			exit(EXIT_FAILURE);
		}
	

	if(argc==5){ //only 2 flags 
		if(strcmp(argv[3], "-d") == 0) {
			hasDir=1;
			hasOut=0;
			searchDir = (char*)malloc(strlen(argv[4])*sizeof(char)+1);
			strcpy(searchDir,argv[4]);
		} else if (strcmp(argv[3], "-o") == 0) {
			hasDir=0;
			hasOut=1;
			outputDir = (char*)malloc(strlen(argv[4])*sizeof(char)+1);
			strcpy(outputDir,argv[4]);
		} else {
			printf("Error: Incorrect parameters\n");
			printf("Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}
	} else if (argc==7){ //all flags present
		if(strcmp(argv[3], "-d") == 0) {
			searchDir = (char*)malloc(strlen(argv[4])*sizeof(char)+1);
			strcpy(searchDir,argv[4]);
			hasDir=1;
		} else {
			printf("Error: Incorrect parameters for 2nd argument\n");
			printf("Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}

		if (strcmp(argv[5],"-o")==0){
			outputDir = (char*)malloc(strlen(argv[6])*sizeof(char)+1);
			strcpy(outputDir,argv[6]);
			hasOut=1;
		} else {
			printf("Error: Incorrect parameters for 3rd argument\n");
			printf("Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
			exit(EXIT_FAILURE);
		}
	} else if (argc>7||argc==4||argc==6){
		if(argc>7){
			printf("Error: Too many parameters\n");
			printf("Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");		
		}	
		else {
			printf("Error: Incorrent arguments\n");
			printf("Format for parameters: -c <colName> -d <directoryName> -o <outputDirectoryName>\nNote: -d <directoryName> and -o <outputDirectoryName> are optional\n");	
		}
		exit(EXIT_FAILURE);
	}
	

	printf("colToSort:\t%s\n",colToSort);
	
	if(hasDir){
		DIR *dp;
		if((dp = opendir(searchDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",searchDir);
	        exit(EXIT_FAILURE);
   		}
   		//printf("searchDir:\t%s\n",searchDir);	
	}

	if(hasOut){
		DIR *dp;
		if((dp = opendir(outputDir)) == NULL) {
	        fprintf(stderr,"Error: cannot open directory: [%s]\n",outputDir);
	        exit(EXIT_FAILURE);
   		}
   		//printf("outputDir:\t%s\n",outputDir);	
	}



	//THIS IS NOT USED
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
           /*funtion is called recursively at a new indent level */
            numP += numProc(entry->d_name);
            //printf("nump after recursing %s:\t%d\n",entry->d_name,numP);
        }
        else if(S_ISREG(statbuf.st_mode)){ //ITS A FILE, FORK TO SORT FILE
            
                numP++;
                
        }
    }

        chdir("..");
        closedir(dp);
        return numP;
}


int numP;
	if(hasDir==0){
		numP = numProc(currDir);
	} else
		numP=numProc(searchDir);



---------------------------------------------
FOR writing to file and shiiiit



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
   		if(c<27){
   			fprintf(sorted,",");
   		}
   		c++;
   	}
   	fprintf(sorted, "\n");	

	writeCSV(frontRec,sorted);


	-----------------------------------------------------------
	from the sort function idk this prints shit

	  		strips the weird character from the last entry and shiiit
  	 		int f =0;
		hNode *ptr1 = headersFront;
 	  		while (ptr1->next!=NULL){
	   		ptr1=ptr1->next;
	   	}
	   	ptr1->data[strlen(ptr1->data)-1] = '\0'; 

		 
		------------------------test headers
		       int f =0;
				hNode *ptr1 = headersFront;
		   		while (ptr1!=NULL){
			   		printf("'%s'",ptr1->data);
			   		ptr1=ptr1->next;
			   		if(f<numCols-1){
			   			printf(",");
			   		}
			   		f++;
		   		}
		   		printf("\n\n\n");	
		---------------------test headers