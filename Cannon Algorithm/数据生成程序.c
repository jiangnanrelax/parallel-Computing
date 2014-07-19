#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define n1 100
#define n2 150
#define n3 250
#define max 1000
int main(){
	int i,j,k;
	FILE *fa,*fb,*fCheck;
	int A[n1][n2],B[n2][n3],CCheck[n1][n3];

	clock_t start, finish; 
	double duration; 

	fa=fopen("a.txt","w");
	fb=fopen("b.txt","w");
	fCheck=fopen("c_check.txt","w");
	if(fa==NULL ||fb==NULL ||fCheck==NULL){
		printf("Can't open file!\n");
		exit(-1);
	}
	srand((unsigned)time(NULL));
	fprintf(fa,"%d %d\n",n1,n2);
	for(i=0;i<n1;i++){
		for(j=0;j<n2;j++){
			A[i][j]=rand()%max;
			fprintf(fa," %d",A[i][j]);
		}
		fprintf(fa,"\n");
	}
	fprintf(fb,"%d %d\n",n2,n3);
	for(i=0;i<n2;i++){
		for(j=0;j<n3;j++){
			B[i][j]=rand()%max;
			fprintf(fb," %d",B[i][j]);
		}
		fprintf(fb,"\n");
	}

	/*generate matrix C using simple method*/
	start=clock();
	memset(CCheck,0,sizeof(CCheck));
	for(i=0;i<n1;i++){
		for(j=0;j<n3;j++){
			for(k=0;k<n2;k++){
				CCheck[i][j]+=A[i][k]*B[k][j];
				
			}
			fprintf(fCheck," %d",CCheck[i][j]);
		}
		fprintf(fCheck,"\n");
	}	
	finish=clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;  
	printf( "%lf seconds\n", duration );  
	fclose(fa);
	fclose(fb);
	fclose(fCheck);

}
