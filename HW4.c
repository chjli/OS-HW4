#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/time.h>

int read_file(char *, int *);
void write_file(int *, int);
void bubble_sort(int *, int);
int partition(int *, int, int);
void *thread(void *arg);

int array[1000001];
sem_t finish;
sem_t lock;
sem_t job_num;

struct job_information{
	int begin;
	int end;
	int kind;	//0 do sort 1 for T1 2 for T2T3 3 for T4T5T6T7
};
struct Queue{
	int first;
	int last;
	struct job_information data[100];
};
struct Queue job_queue;

void push_job(int kind, int p, int r)
{
	sem_wait(&lock);
	
	job_queue.data[job_queue.last].begin = p;
	job_queue.data[job_queue.last].end = r;
	job_queue.data[job_queue.last].kind = kind;	
	job_queue.last++;
	
	sem_post(&lock);
	sem_post(&job_num);
}

struct job_information pop_job()
{
	sem_wait(&job_num);
	sem_wait(&lock);
	
	struct job_information job;
	job.begin = job_queue.data[job_queue.first].begin;
	job.end = job_queue.data[job_queue.first].end; 	
	job.kind = job_queue.data[job_queue.first].kind; 	
	job_queue.first++;
	
	sem_post(&lock);
	return job;
} 

void work(struct job_information job)
{
	if(job.kind == 0)
	{
		bubble_sort(array + job.begin, job.end - job.begin + 1);
		sem_post(&finish);
	}
	else if(job.kind == 1)
	{		
		int middle = partition(array, job.begin, job.end);
		push_job(2, job.begin, middle - 1);
		push_job(2, middle + 1, job.end);
	}	
	else if(job.kind == 2)
	{		
		int middle = partition(array, job.begin, job.end);
		push_job(3, job.begin, middle - 1);
		push_job(3, middle + 1, job.end);
	}	
	else if(job.kind == 3)
	{		
		int middle = partition(array, job.begin, job.end);
		push_job(0, job.begin, middle - 1);
		push_job(0, middle + 1, job.end);
	}
}

int main(int argc, char *argv[])
{
	char input[1000];
	printf("Please enter the name of input file : ");
	fscanf(stdin, "%s", input);
	int num = read_file(input, array);
	int num_t, i;
	printf("Please enter the size of thread pool : ");	
	fscanf(stdin, "%d", &num_t);
	
	struct timeval start, end;	
	gettimeofday(&start, 0);	
	///////
	job_queue.first = 0;
	job_queue.last = 0;
	pthread_t t[num_t];
	sem_init(&finish, 0, 0);
	sem_init(&job_num, 0, 0);
	sem_init(&lock, 0, 1);
	for(i=0; i<num_t; i++)
		pthread_create(&t[i], NULL, thread, NULL);
	push_job(1, 0, num-1);
	int wait = 8;
	while(wait--)								//wait for sorting
		sem_wait(&finish);		
	///////
	gettimeofday(&end, 0);
	
	int sec = end.tv_sec - start.tv_sec;
	int usec = end.tv_usec - start.tv_usec;
	printf("Processing time for thread pool having %d threads : %f sec. \n", num_t, sec+(usec/1000000.0));
	
	write_file(array, num);
	
	return 0;
}

void *thread(void *argument){
	while(1)
	{
		struct job_information job = pop_job();
		work(job);	
	}
}

int read_file(char *input, int *ary){
	FILE *fp;
	fp = fopen(input, "r");
	if(!fp)
	{
		printf("Cannot open file: %s\n", input);
		exit(1);
	}
	int i, num;
	while(!feof(fp))
	{
		fscanf(fp, "%d", &num);
		for(i=0; i<num; i++)
		{
			fscanf(fp, "%d ", &ary[i]);
		}			
	}
	fclose(fp);
	return num;			
}
void write_file(int *ary, int num){
	FILE *fp;
	fp = fopen("output.txt","w");
	if(!fp)
	{
		printf("Cannot open output.txt\n");
		exit(1);
	}
	int i;
	for(i=0; i<num; i++)
		fprintf(fp, "%d ", ary[i]);
	fclose(fp);
}


int partition(int *array, int p, int r){
	int pivot = array[r];
	while(p < r) {
		while(array[p] < pivot) {
			p++;
		}
		while(array[r] > pivot) {
			r--;
		}
		if(array[p] == array[r])
			p++;
		else if(p < r) {
			int tmp = array[p];
			array[p] = array[r];
			array[r] = tmp;
		}
	}
	return r;
}
void bubble_sort(int *ary, int num){
	int i, j, temp;
	for(i=0; i<num-1; i++)	
		for(j=0; j<num-i-1; j++)
		if(ary[j] > ary[j+1])
		{
			temp = ary[j];
			ary[j] = ary[j+1];
			ary[j+1] = temp;
		}	
}
