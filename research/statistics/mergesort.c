#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mergesort(int a[], int low, int high){
	int i=0;
	int length=high-low+1;
	int pivot=0;
	int merge1=0;
	int merge2=0;
	int worker[length];

	if(low==high)
		return;

	pivot=(low+high)/2;

	mergesort(a,low,pivot);
	mergesort(a,pivot+1,high);

	for(i=0;i<length;i++){
		worker[i]=a[low+i];
	}
	merge1=0;
	merge2=pivot-low+1;
	for(i=0;i<length;i++){
		if(merge2<=high-low){
			if(merge1<=pivot-low){
				if(worker[merge1]>worker[merge2]){
					a[i+low]=worker[merge2++];
				}else{
					a[i+low]=worker[merge1++];
				}
			}else{
				a[i+low]=worker[merge2++];
			}
		}else{
			a[i+low]=worker[merge1++];
		}
	}
}

char * myreadline(){
	char* input=NULL;
	size_t inputlength=0;
	if(getline(&input,&inputlength,stdin)!=-1){
		return input;
	}else{
		return NULL;
	}
}
int parse(char * line, int idx){
	char* pick;
	int i=1;
	pick=strtok(line," (,)");
	while(pick!=NULL){
		i++;
		pick=strtok(NULL," (,)");
		if(i==idx)
			break;
	}
	int num=atoi(pick);
	return num;
}

int main(int argc, char** argv){
	if(argc!=2){
		printf("Usage: %s <the postion index (e.g. 1)>\n", *argv);
		return 0;
	}
	int idx=strtol(*(argv+1),NULL,10);
	int a[10000];	
	char* input=NULL;
	int k=0;
	while(input=myreadline()){
		a[k]=parse(input,4+idx);
		k++;
		free(input);
	}
	mergesort(a, 0,9999);
	int i=0;
	for(;i<10000;i++)
		printf("%d,",a[i]);
	return 0;
}
