#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define MAXN 1000
int tag=333;
int max_length=500;
MPI_Status status;
int comp(const void*p,const void*q){
	return(*(int*)p - *(int*)q);
}
void generate_data(int *data_list,int size){
	int i;
	srand(time(NULL));
	for(i=0;i<size;i++){
		data_list[i]=rand()%MAXN;
	}
}
void send_data(int* source,int p,int len){
	int i;
	for(i=1;i<=p;i++){
		MPI_Send(source+(i-1)*len,len,MPI_INT,i,tag,MPI_COMM_WORLD);
	}
}
void receive_data(int *sample,int sample_len){
	MPI_Recv(sample,sample_len,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
	qsort(sample,sample_len,sizeof(int),comp);
	printf("data received\n");
}
void select_represent(int *sample,int len,int w,int p){
	int i=0,count=0;
	int* tmp=(int*)malloc(p*sizeof(int));
	for( i=0;count<=p&&i<len;i+=w){
		tmp[count]=sample[i];
		count++;
	}
	MPI_Send(tmp,p,MPI_INT,0,tag+1,MPI_COMM_WORLD);
}
int* fetch_represent(int p){
	int len=p*p,i,*tmp;
	int *main_item;
	main_item=(int*)malloc((p-1)*sizeof(int));
	int count=0;
	tmp=(int*)malloc(len*sizeof(int));
	printf("debuging\n");
	for( i=1;i<=p;i++){
		MPI_Recv(tmp+p*(i-1),p,MPI_INT,i,tag+1,MPI_COMM_WORLD,&status);
	}
	qsort(tmp,len,sizeof(int),comp);
	for(i=p;i<len;i+=p){
		main_item[count++]=tmp[i];
		/*printf("main_items[%d]:%d\n",count-1,main_item[count-1]);*/
	}	
	return main_item;
}
void send_main_item(int *main_item,int len,int p){
	int i=0;
	for(i=1;i<=p;i++){
		MPI_Send(main_item,len,MPI_INT,i,tag+1,MPI_COMM_WORLD);
	}
}
void receive_main_item(int *items,int len){
	int i;	
	MPI_Recv(items,len,MPI_INT,0,tag+1,MPI_COMM_WORLD,&status);
	for(i=0;i<len;i++){
		printf("%d  ",items[i]);
	}
}
void split(int*sample,int len,int*items,int len_p){
	int start=0,end=0,i,count=0;
	for(i=0;i<len;i++){
		/*printf("sample[%d]:%d\n",i,sample[i]);*/
	}
	for(i=0;i<len_p;i++){
		/*printf("items[%d]:%d\n",i,items[i]);*/
	}
	for(i=0;i<len_p&&end<len;){
		/*printf("items[%d]:%d Vs sample[%d]:%d\n",i,items[i],end,sample[end]);*/
		if(items[i]<sample[end]){
			int j;
			MPI_Send(sample+start,end-start,MPI_INT,i+1,tag+2,MPI_COMM_WORLD);
			/*printf("\nstart:%d,end:%d\n",start,end);
                	for(j=start;j<end;j++)
                        	printf("%d  ",sample[j]);*/
			start=end;
			i++;
		}
		else{
			end++;
		}
	}
	

	if(end<len){
		int i;
		MPI_Send(sample+end,len-end,MPI_INT,len_p+1,tag+2,MPI_COMM_WORLD);
	}
}
void fetch_merge(int numprocs){
	int len=0,count=0,curlen=0,fin_cnt=0;
	int *sorted,*tmp;
	int i,j;
	int* splited_items=(int*)malloc(max_length*sizeof(int));
	sorted=(int*)malloc(max_length*sizeof(int));
	tmp=(int*)malloc(max_length*sizeof(int));

	for(i=0;i<max_length;i++){
		tmp[i]=MAXN;
	}

	
	printf("***I put an egg***\n");
	for(i=1;i<numprocs;i++){
		int a=0,b=0;
		memset(splited_items,0,sizeof(splited_items));
		MPI_Recv(splited_items,max_length,MPI_INT,i,tag+2,MPI_COMM_WORLD,&status);
		for(j=0,count=0;j<max_length&&splited_items[j]!=0;j++)
			count++;
		
		/*printf("current count:%d\n",count);*/
		for(a=0,b=0,fin_cnt=0;a<count&&b<curlen;){
			sorted[fin_cnt++]=(tmp[a]<splited_items[b]?tmp[a++]:splited_items[b++]);
			/*printf("sorted[%d]:%d\n",fin_cnt-1,sorted[fin_cnt-1]);*/
		}
		while(a<count) sorted[fin_cnt++]=tmp[a++];
		while(b<curlen) sorted[fin_cnt++]=tmp[b++];
		for(j=0;j<curlen;j++)
			tmp[j]=sorted[j];
		curlen+=count;
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Send(sorted,curlen,MPI_INT,0,tag+3,MPI_COMM_WORLD);
}
void collect_result(int p){
	int i,j;
	int *tmp=(int*)malloc(max_length*sizeof(int));
	for(i=1;i<=p;i++){
		memset(tmp,0,sizeof(tmp));
		MPI_Recv(tmp,max_length,MPI_INT,i,tag+3,MPI_COMM_WORLD,&status);
		printf("\nI got the final result!\n");
		for(j=0;j<max_length&&tmp[j]!=0;j++)
			printf("tmp[%d]: %d  ",j,tmp[j]);
	}
}		
		
int main(int argc,char* argv[]){
	int *source,*sample;
	int size=1000,p;
	int myid,numprocs;
	int sample_len;
	int w;
	int * main_item,*items;
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	p=numprocs-1;
	sample_len=size/p;
	w=sample_len/p;
	
	if(myid==0){
		source=(int *)malloc(size*sizeof(int));
		generate_data(source,size);
		send_data(source,p,sample_len);
	}else{
		sample=(int*)malloc(sample_len*sizeof(int));
		receive_data(sample,sample_len);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(myid==0){
		int i=0;
		main_item=fetch_represent(p);
		/*printf("debugging main_items returned failed?\n");
		for(i=0;i<p-1;i++){
			printf("%d ",main_item[i]);
		}*/
		send_main_item(main_item,p-1,p);
	}else{
		items=(int*)malloc((p-1)*sizeof(int));
		select_represent(sample,sample_len,w,p);
	/*	printf("***this is procs:%d***\n",myid);*/
		receive_main_item(items,p-1);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(myid!=0){
		split(sample,sample_len,items,p-1);
		printf("this is threads:%d",myid);
		/*fetch_merge(numprocs);*/
		printf("\n");
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(myid!=0){
		fetch_merge(numprocs);
		MPI_Barrier(MPI_COMM_WORLD);
		
	}
	MPI_Barrier(MPI_COMM_WORLD);	
	if(myid==0){
		printf("I am your admin\n");
		collect_result(p);
	}
	
	MPI_Finalize();
}
