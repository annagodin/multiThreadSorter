#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "multiThreadSorter_thread.h"

//converts a string to lower case
char* toLowerCase (char* str){
	
	int i;
	for(i=0; i<strlen(str); i++){
		str[i] = tolower(str[i]);
	}
	return str;
}

/*is_digit(): decides if its a digit/string/decimal */
int is_digit(char*str){

	int i;
	int isDec = 0;

	for(i = 0; i < strlen(str); i++){
		
		if((!isdigit(str[i])) && str[i] != '.'){
			//printf("the string is not a number. \n");
			return 0;		//string is a word
		}
		
		if(str[i] == '.'){
			//printf("the string has a period in it. \n");
			isDec = 1;
		}

		if(isdigit(str[i])){
			//printf("str[%d] val is: %c\n", i, str[i]);
			continue;
		}

	}
	
	if(isDec == 1){
		//printf("the string is a decimal. \n");
		return 2;		//string is decimal value
	}
	else if(isDec == 0){
		//printf("the string is an int. \n");
		return 1;		//integer
	}
}


//compares two fields and outputs: (1 if a>b, -1 if a<b, -1 if a=b)
int compareFields(char* a, char*b){
	//printf("comparing '%s' and '%s'\n",a,b);
	if (a==NULL){
		//printf("%s is less than %s\n\n",a,b);
		return -1;
	} else if (b==NULL){
		//printf("%s is greater than %s\n\n",a,b);
		return 1;
	}
	if(is_digit(a)!=0 && is_digit(b)!=0) { //not a string
		if(is_digit(a)==1 && is_digit(b)==1){ //integer
			//printf("integers\n");
			int numA = atoi(a);
			int numB = atoi(b);
			//printf("a is %d and b is %d\n",numA,numB);
			if (numA<=numB){
				//printf("%d is less than %d\n\n",numA,numB);
				return -1;
			}else{
				//printf("%d is greater than %d\n\n",numA,numB);
				return 1;
			}
		} else { //decimal
			//printf("decimal\n");
			char *ptr;
			double numA = strtod(a,&ptr);
			double numB = strtod(b,&ptr);
			//printf("a is %lf and b is %lf\n",numA,numB);
			if (numA<=numB){
				//printf("%lf is less than %lf\n\n",numA,numB);
				return -1;
			} else {
				//printf("%lf is greater than %lf\n\n",numA,numB);
				return 1;
			}	
			//strtod(str,&ptr)
		}
	} else{//string
		//printf("string\n");
		//printf("a before: %s\tb before: %s\n",a,b);

		if(a[0]=='"' /*&& a[strlen(a)-1]=='"'*/){
			//printf("A: function to strip quote\n");
			a=stripFirstChar(a, strlen(a));
			a=stripLastChar(a);
		}			
		a= toLowerCase(a);
		a=trimWhiteSpace(a);
		//printf("this is a:\t%s\n",a);

		if(b[0]=='"' /*&& b[strlen(b)-1]=='"'*/){
			//printf("B function to strip quote\n");
			b=stripFirstChar(b, strlen(b));
			b=stripLastChar(b);
		}
		b=toLowerCase(b);
		b=trimWhiteSpace(b);
		//printf("this is b:\t%s\n",b);
		//printf("a is '%s' and b is '%s'\n",a,b);
		if(strcmp(a,b)<0 || strcmp(a,b)==0){
			//printf("%s is less than %s\n\n",a,b);
			return -1;
		} else{
			//printf("%s is greater than %s\n\n",a,b);
			return 1;
		}
	}
	
}

//sorts the linked list by changing next pointers (not data)
void mergesort(CSVrecord** headRef) {
    CSVrecord* head = *headRef;
    CSVrecord* a;
    CSVrecord* b;

    //Base case
    if ((head == NULL) || (head->next == NULL))
    {
        return;
    }

    //split head list into sublists, a and b
    FrontBackSplit(head, &a, &b);

    //Recursively sort the sublists
    mergesort(&a);
    mergesort(&b);

    //merge the two sorted lists together
    *headRef = SortedMerge(a, b);
}

//merges two sorted linked lists
CSVrecord* SortedMerge(CSVrecord* a, CSVrecord* b) {
    
    CSVrecord* result = NULL;

    //Base cases
    if (a == NULL)
        return(b);
    else if (b==NULL)
        return(a);

    //compares the fields, and then recurs based on vals
    if (compareFields(a->sortVal, b->sortVal)==-1){
        result = a;
        result->next = SortedMerge(a->next, b);
    }
    else {
        result = b;
        result->next = SortedMerge(a, b->next);
    }
    return(result);
}


//Split into front and back halves, return the two lists
//If the length is odd, extra node goes in the front list.
 //fast/slow pointer strategy
void FrontBackSplit(CSVrecord* source, CSVrecord** frontRef, CSVrecord** backRef){
    CSVrecord* fast;
    CSVrecord* slow;
    slow = source;
    fast = source->next;

    //Advance 'fast' two nodes, and advance 'slow' one node
    while (fast != NULL){
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    //'slow' is before the midpoint in the list, so split it in two at that point.
    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
    
}


