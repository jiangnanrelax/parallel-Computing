#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define n1 100
#define n3 250
int main(){
	int i,j,k;
	int flag=0;
	FILE *fc,*fCheck;
	int C[n1][n3],CCheck[n1][n3];

	
	fc=fopen("c.txt","r");
	fCheck=fopen("c_check.txt","r");
	if(fc==NULL||fCheck==NULL){
		printf("Can't open file!\n");
		exit(-1);
	}
	
	for(i=0;i<n1;i++){
		for(j=0;j<n3;j++){
			fscanf(fc,"%d ",&C[i][j]);
			fscanf(fCheck,"%d ",&CCheck[i][j]);
			
			if(C[i][j]!=CCheck[i][j]){
				printf("%d %d\n",C[i][j],CCheck[i][j]);
				flag=1;
			}
		}
	}
	if(flag==0){
		printf("Designed Cannon Algorithm is right!\n");
	}else{
		printf("Designed Cannon Algorithm is wrong!\n");
	}
	fclose(fc);
	fclose(fCheck);
		
}

