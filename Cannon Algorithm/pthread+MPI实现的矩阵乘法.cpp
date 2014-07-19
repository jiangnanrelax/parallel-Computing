#include"mpi.h"
#include"pthread.h"
#include<stdio.h>
#include<stdlib.h>
#include <sys/sysinfo.h>
#define M 100
#define N 150
#define P 250
/*创建struct 便于管理信息*/
struct threadArg{
	int tid;
	int (*B)[P];    /*二维矩阵，用指针数组保存每一行的第一个数的地址*/
	int *A_row;     /*矩阵A的一行*/
	int *C_row;     /*结果矩阵C的一行*/
	int numthread;
};
void *worker(void*arg){
	int i,j;
	struct threadArg *myarg=(struct threadArg*)arg;
	/*平均分配D的所有列*/
	for(i=myarg->tid;i<P;i+=myarg->numthread){
		myarg->C_row[i]=0.0;
		for(j=0;j<N;j++){
			/*B的一列与A的一行相乘，存入C对应位置中*/
			myarg->C_row[i]+=myarg->A_row[j]*myarg->B[j][i];
		}
		/*printf("C[%d]:%f\n",i,myarg->C_row[i]);*/
	}
	return NULL;
}
int main(int argc,char*argv[]){
	double start_time=0,end_time=0;
	int i,j;
	int myid,numprocs;
	MPI_Status status;
	int sender;
	int A[M][N],B[N][P],C[M][P];
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

	if(myid==0){
		int n1,n2,n3;
		
		FILE *file_a,*file_b;
		file_a=fopen("a.txt","r");
		file_b=fopen("b.txt","r");
		fscanf(file_a,"%d %d",&n1,&n2);
		fscanf(file_b,"%d %d",&n2,&n3);
		for(int i=0;i<n1;i++)
			for(int j=0;j<n2;j++)
				fscanf(file_a,"%d",&A[i][j]);
			
		for(int i=0;i<n2;i++)
			for(int j=0;j<n3;j++)
				fscanf(file_b,"%d",&B[i][j]);
		fclose(file_a);
		fclose(file_b);	
	}
	/*将矩阵B广播给所有其他计算节点*/
	MPI_Bcast(B[0],N*P,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);	

	if(myid==0){                 /*分配任务，回收结果*/
		int i,j,numsend;
		/*比较进程数量和A的行维数大小，选取二者之中的较小者*/
		start_time=MPI_Wtime();
		j=((numprocs-1)<M?(numprocs-1):M);
		for(i=1;i<=j;i++){
			/*将A的每一行分发给从进程*/
			MPI_Send(A[i-1],N,MPI_INT,i,99,MPI_COMM_WORLD);
		}

		numsend=j;
		for(i=1;i<=M;i++){
			sender=(i-1)%(numprocs-1)+1;
			if(numsend<M){/*若数组A还没有发送完数据，则继续发送*/
				MPI_Send(A[i-1],N,MPI_INT,sender,99,MPI_COMM_WORLD);
				numsend++;
			}else{
				/*终止sender进程的运行*/
				MPI_Send(&j,0,MPI_INT,sender,0,MPI_COMM_WORLD);
			}
			/*回收来自从进程sender的计算结果*/
			MPI_Recv(C[i-1],P,MPI_INT,sender,100,MPI_COMM_WORLD,&status);
		
			
		}
		end_time=MPI_Wtime()-start_time;
		printf("matrix multiply: multiply a %d* %d with a %d*%d, use %f(s)\n",
			M,N,N,P,end_time);
		/*打印结果*/
		FILE *file_C=fopen("d.txt","w");

		printf("result:\n");
		for(i=0;i<M;i++){
			for(j=0;j<P;j++){
				fprintf(file_C,"%d ",C[i][j]);
			}
			fprintf(file_C,"\n");
		}
		fclose(file_C);
		
	}else{
		int i,j;
		int numthread=get_nprocs();/*获取当前计算节点的CPU数量*/
		
		printf("cpu number:%d.\n",numthread);/*get cpu number*/
		/*创建一组线程，保证每个CPU上运行一个线程*/
		pthread_t *tids=(pthread_t*)malloc(numthread*sizeof(pthread_t));
		/*A矩阵的一行*/
		int *A_row=(int*)malloc(N*sizeof(int));
		/*C矩阵的一行*/
		int *C_row=(int*)malloc(P*sizeof(int));
		/*用于统一管理每个线程的A，B，C矩阵信息*/
		struct threadArg *targs=(struct threadArg*)malloc(numthread*sizeof(struct threadArg));
		for(i=0;i<numthread;i++){
			targs[i].tid=i;    /*编号*/
			targs[i].B=B;
			targs[i].A_row=A_row;
			targs[i].C_row=C_row;
			targs[i].numthread=numthread;
		}
		
		while(1){
			int i,j;
			/*接收主进程发来的一行,MPI_ANY_TAG表示任意标记的数据都要接收*/
			MPI_Recv(A_row,N,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			/*for(i=0;i<N;i++)
				printf("A_row[%d]: %.0f \n",i,A_row[i]);*/
			/*检查接受到的tag标记，若为0，则退出*/
			if(status.MPI_TAG==0) break;
			for(i=0;i<numthread;i++){
				/*创建一组线程
				第一个参数为指向线程ID。
				第二个参数用来设置线程属性。
				第三个参数是线程运行函数的起始地址。
				最后一个参数是运行函数的参数*/
				pthread_create(&tids[i],NULL,worker,&targs[i]);
			}
			for(i=0;i<numthread;i++){
				/*以阻塞的方式等待thread指定的线程结束,等待全部线程计算完成*/
				pthread_join(tids[i],NULL);
			}
			/*从进程返回计算结果给0进程*/
			MPI_Send(C_row,P,MPI_INT,0,100,MPI_COMM_WORLD);
			/*printf("I put an flag\n");
			for(j=0;j<P;j++){
				printf("%d ",(int)C_row[j]);
			}
			printf("I put an flag again\n");*/
		}
	}
	MPI_Finalize();
	return 0;

}
