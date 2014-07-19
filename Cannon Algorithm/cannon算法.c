#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*It'OK!*/
void scatter_matrix(int* fstream,int n1,int n2,int*Q,int root,int tag){
	int rows=(n1+root-1)/root;
	int cols=(n2+root-1)/root;
	int* tmp_matrix=(int*)malloc(rows*cols*sizeof(int));
	
	int i,j;
	memset(Q,0,rows*cols*sizeof(int));
	for(i=0;i<root;i++){
		for(j=0;j<root;j++){
			int p=0,q=0;
			int imin=i*rows*n2;
			int jmin=j*cols;
			memset(tmp_matrix,0,sizeof(tmp_matrix));
			for(p=0;p<rows;p++,imin+=n2){
				for(q=0;q<cols;q++){
					tmp_matrix[p*cols+q]=fstream[imin+jmin+q];
				}
			}
			if(i==0&&j==0){
				memcpy(Q,tmp_matrix,rows*cols*sizeof(int));
			}else{
				MPI_Send(tmp_matrix,rows*cols,MPI_INT,i*root+j,tag,MPI_COMM_WORLD);
			}	

			/*printf("I send data to %d\n",i*root+j);
			for(p=0;p<rows;p++,imin+=n2){
				for(q=0;q<cols;q++){
					printf("%d ",tmp_matrix[p*cols+q]);
				}
				printf("\n");
			}*/
		}
	}
	/*printf("I had scatter data\n");*/
}
int get_index(int row,int col,int sp){
	int tmp=((row+sp)%sp)*sp+(col+sp)%sp;
	/*printf("row:%d,col:%d,index:%d\n",row,col,tmp);*/
	return tmp;
}
void matrix_multi(int* A,int *B,int *C,int n1,int n2,int n3,int myid){
	int i=0,j=0,k=0;
	int* tmp_C=(int*)malloc(n1*n3*sizeof(int));
	memset(tmp_C,0,sizeof(int)*n1*n3);
	
	/*printf("I am proc:%d,matrix C: ",myid);*/
	for(i=0;i<n1;i++){
		for(j=0;j<n3;j++){
			for(k=0;k<n2;k++){
				tmp_C[i*n3+j]+=A[i*n2+k]*B[k*n3+j];
			}
		}
	}

	for(i=0;i<n1*n3;i++){
		C[i]+=tmp_C[i];
	}
	/*if(myid!=20)
		return;
	printf("I am orig proc:%d  ",myid);

	printf("matrix A: \n");
	for(i=0;i<n1;i++){
		printf("%d,A:    ",myid);
		for(j=0;j<n2;j++){
			printf("%d ",A[i*n2+j]);
		}
		printf("\n");
	}
	printf("matrix B: \n");
	for(i=0;i<n2;i++){
		printf("B,%d:    ",myid);
		for(j=0;j<n3;j++){
			printf("%d ",B[i*n3+j]);
		}
		printf("\n");
	}
	printf("matrix C: \n");
	for(i=0;i<n1;i++){
		printf("%d,C:      ",myid);
		for(j=0;j<n3;j++){
			printf("%d ",tmp_C[i*n3+j]);
		}
		printf("\n");
	}*/
}
void shuffle(int*A,int*buf_A,int buf_A_size,int *B,int*buf_B,int buf_B_size,int root,int myid){
	int i,j;
	MPI_Status status;
	int cur_col=0;
	int cur_row=0;
	cur_row=myid/root;
	cur_col=myid-cur_row*root;
	int len=cur_row;
	printf("myid:%d ,cur_row:%d ,cur_col:%d\n",myid,cur_row,cur_col);
	for(i=0;i<len;i++){
		MPI_Sendrecv(A,buf_A_size,MPI_INT,get_index(cur_row,cur_col-1,root),102,
			     buf_A,buf_A_size,MPI_INT,get_index(cur_row,cur_col+1,root),102,MPI_COMM_WORLD,&status);
		memcpy(A,buf_A,buf_A_size*sizeof(int));
		memset(buf_A,0,buf_A_size*sizeof(int));	
	}	
	len=cur_col;
	for(j=0;j<len;j++){
		MPI_Sendrecv(B,buf_B_size,MPI_INT,get_index(cur_row-1,cur_col,root),103,
			     buf_B,buf_B_size,MPI_INT,get_index(cur_row+1,cur_col,root),103,MPI_COMM_WORLD,&status);
		memcpy(B,buf_B,buf_B_size*sizeof(int));
		memset(buf_B,0,buf_B_size*sizeof(int));
	}
	printf("I have shuffled!\n");
}
void cannon(int*A,int*buf_A,int buf_A_size,int *B,int*buf_B,int buf_B_size,
	int *C,int buf_C_size,int row_a,int col_a,int col_b,int root,int myid){
	MPI_Status status;
	int i,j;
	/*printf("myid:%d ,cur_row:%d ,cur_col:%d\n",myid,cur_row,cur_col);*/
	memset(buf_A,0,buf_A_size*sizeof(int));
	memset(buf_B,0,buf_B_size*sizeof(int));
	/*printf("buf_A_size:%d,buf_B_szie:%d,buf_C_size:%d\n",buf_A_size,buf_B_size,buf_C_size);*/
	memset(C,0,sizeof(int)*buf_C_size);
	int cur_col=0;
	int cur_row=0;
	cur_row=myid/root;
	cur_col=myid-cur_row*root;
	for(i=0;i<root;i++){
		matrix_multi(A,B,C,row_a,col_a,col_b,myid);		
		MPI_Sendrecv(A,buf_A_size,MPI_INT,get_index(cur_row,cur_col-1,root),102,
			     buf_A,buf_A_size,MPI_INT,get_index(cur_row,cur_col+1,root),102,MPI_COMM_WORLD,&status);
		memcpy(A,buf_A,buf_A_size*sizeof(int));
		memset(buf_A,0,buf_A_size*sizeof(int));
		MPI_Sendrecv(B,buf_B_size,MPI_INT,get_index(cur_row-1,cur_col,root),103,
			     buf_B,buf_B_size,MPI_INT,get_index(cur_row+1,cur_col,root),103,MPI_COMM_WORLD,&status);
		memcpy(B,buf_B,buf_B_size*sizeof(int));
		memset(buf_B,0,buf_B_size*sizeof(int));
		/*printf("I am proc %d,round:%d\n",myid,i);
		for(j=0;j<col_a;j++){
			printf("%d, %d:      ",i,myid);
			for(k=0;k<col_b;k++){
				printf("%d ",B[j*col_b+k]);
			}
			printf("\n");
		}*/
	}
		/*printf("I am orig proc:%d  ",myid);
		for(i=0;i<row_a;i++){
			for(j=0;j<col_b;j++){
				printf("%d ",C[i*col_b+j]);
			}
			printf("\n");
		}*/
	
	MPI_Send(C,row_a*col_b,MPI_INT,0,104,MPI_COMM_WORLD);
	if(myid!=20)
		return;
	/*printf("spec,I am proc %d\n",myid);
	for(i=0;i<row_a;i++){
		printf("%d:      ",myid);
		for(j=0;j<col_b;j++){
			printf("%d ",C[i*col_b+j]);
		}
		printf("\n");
	}*/
}
 
void gather_matrix(int *fstream,int n1,int n3,int*C,int root,FILE*fhc){
	MPI_Status status;
	int rows=(n1+root-1)/root;
	int cols=(n3+root-1)/root;
	int* tmp_matrix=(int*)malloc(rows*cols*sizeof(int));
	int i,j;

	for(i=0;i<root;i++){
		for(j=0;j<root;j++){
			int p,q;
			int imin=i*rows*n3;
			int jmin=j*cols;
			memset(tmp_matrix,0,sizeof(tmp_matrix));
			MPI_Recv(tmp_matrix,rows*cols,MPI_INT,i*root+j,104,MPI_COMM_WORLD,&status);

			
			printf("I am passed proc:%d \n",i*root+j);
			for(p=0;p<rows;p++,imin+=n3){
				for(q=0;q<cols;q++){
					fstream[imin+jmin+q]=tmp_matrix[p*cols+q];
					/*printf("%d ",((int*)fstream)[imin+jmin+q]);*/
				}
				/*printf("\n");*/
			}
		}
	}

	for(i=0;i<n1;i++){
		for(j=0;j<n3;j++){
			fprintf(fhc,"%d ",((int*)fstream)[i*n3+j]);
		}
		fprintf(fhc,"\n");
	}
}

int main(int argc,char**argv){
	int myid,numprocs;
	int i,j;
	MPI_Status status;
	int root=0;
	int dim[3];
	double elapsed_time=0;
	int max_rows_a,max_cols_a,max_rows_b,max_cols_b;
	int buf_A_size,buf_B_size,buf_C_size;
	FILE* fhc;
	/*suppose A:n1*n2 ,B:n2*n3;n1,n2,n3 are read from input file*/
	int n1,n2,n3;
	/*buffer for matrix A,B,C will be shifted ,so they each have two buffer*/
	int *A,*B,*C,*buf_A,*buf_B;

	/*on proc0,buffers to cache matrix files of A,B and C*/
	int *fstream_a=NULL,*fstream_b=NULL,*fstream_c=NULL;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

	root=sqrt(numprocs);
	if(numprocs!=root*root){
		printf("process number must be a squre!\n");
	}

	/*on proc0,preprocess the command line,read in file
	 for A,B and put their sizes in dim[]*/
	if(myid==0){
		FILE *file_a,*file_b,*file_c;
		int n1,n2,n3;
		int i,j;
		file_a=fopen(argv[1],"r");
		file_b=fopen(argv[2],"r");
		fscanf(file_a,"%d %d",&n1,&n2);
		fscanf(file_b,"%d %d",&n2,&n3);
	
		dim[0]=n1,dim[1]=n2,dim[2]=n3;
		fstream_a=(int*)malloc(n1*n2*sizeof(int));
		fstream_b=(int*)malloc(n2*n3*sizeof(int));
		/*printf("Yeah! I got n1=%d,n2=%d,n3=%d\n",n1,n2,n3);*/
		for(i=0;i<n1;i++)
			for(j=0;j<n2;j++)
			fscanf(file_a,"%d",&((int*)fstream_a)[i*n2+j]);
			
		for(i=0;i<n2;i++)
			for(j=0;j<n3;j++)
				fscanf(file_b,"%d",&((int*)fstream_b)[i*n3+j]);
	}
	MPI_Bcast(dim,3,MPI_INT,0,MPI_COMM_WORLD);
	n1=dim[0],n2=dim[1],n3=dim[2];

	/*begin new version*/
	max_rows_a=(n1+root-1)/root;
	max_cols_a=(n2+root-1)/root;
	max_rows_b=max_cols_a;
	max_cols_b=(n3+root-1)/root;
	buf_A_size=max_rows_a*max_cols_a;
	buf_B_size=max_rows_b*max_cols_b;
	buf_C_size=max_rows_a*max_cols_b;


	A=(int*)malloc(sizeof(int)*buf_A_size);
	buf_A=(int*)malloc(sizeof(int)*buf_A_size);
	B=(int*)malloc(sizeof(int)*buf_B_size);
	buf_B=(int*)malloc(sizeof(int)*buf_B_size);
	C=(int*)malloc(sizeof(int)*buf_C_size);
	if(A==NULL||buf_A==NULL||B==NULL||buf_B==NULL||C==NULL)
	{
		printf("Memory allocation failed!\n");
		exit(-1);
	}

	/*proc 0 scatter A,B to other procs in a 2D block distribution fashion*/
	if(myid==0){
		/*printf("max_rows_a:%d\n",max_rows_a);
		printf("max_cols_a:%d\n",max_cols_a);
		printf("max_rows_b:%d\n",max_rows_b);
		printf("max_cols_b:%d\n",max_cols_b);*/
		
		scatter_matrix((int*)fstream_a,n1,n2,A,root,100);
		/*printf("I am debuging!\n");*/
		scatter_matrix((int*)fstream_b,n2,n3,B,root,101);
		/*printf("I am finding fault!\n");*/
	}else{
		MPI_Recv(A,max_rows_a*max_cols_a,MPI_INT,0,100,MPI_COMM_WORLD,&status);
		MPI_Recv(B,max_rows_b*max_cols_b,MPI_INT,0,101,MPI_COMM_WORLD,&status);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*printf("I am proc %d\n",myid);
	for(i=0;i<max_rows_a;i++){
		printf("%d:      ",myid);
		for(j=0;j<max_cols_a;j++){
			printf("%d ",A[i*max_cols_a+j]);
		}
		printf("\n");
	}
	printf("I am proc %d\n",myid);
	for(i=0;i<max_rows_b;i++){
		printf("%d:      ",myid);
		for(j=0;j<max_cols_b;j++){
			printf("%d ",B[i*max_cols_b+j]);
		}
		printf("\n");
	}*/

	elapsed_time=MPI_Wtime();
	/*compute C=A*B by Cannon algorithm*/
	shuffle(A,buf_A,buf_A_size,B,buf_B,buf_B_size,root,myid);
	cannon(A,buf_A,buf_A_size,B,buf_B,buf_B_size,
		C,buf_C_size,max_rows_a,max_cols_a,max_cols_b,root,myid);
	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time=MPI_Wtime()-elapsed_time;
	

	MPI_Barrier(MPI_COMM_WORLD);


	int fsize_c=sizeof(int)*n1*n3;
	if(myid==0){
		if(!(fhc=fopen(argv[3],"w"))){
			printf("Cant't open file %s\n",argv[3]);
			MPI_Finalize();
		}
		fstream_c=(int*)malloc(fsize_c);
		gather_matrix(fstream_c,n1,n3,C,root,fhc);
	}
	
	MPI_Barrier(MPI_COMM_WORLD);    /*make sure proc 0 read all it needs*/

	if(myid==0){
		int i,j;
		printf("Cannon algorithm :multiply a %d* %d with a %d*%d, use %f(s)\n",
			n1,n2,n2,n3,elapsed_time);
		/*printf("I have finished!\n");*/
		fclose(fhc);
		
		free(fstream_a);
		free(fstream_b);
		free(fstream_c);
	}

	free(A);free(buf_A);
	free(B);free(buf_B);
	free(C);
	MPI_Finalize();
	return 0;
}