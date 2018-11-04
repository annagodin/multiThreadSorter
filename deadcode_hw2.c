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