#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/mman.h>

#include <string.h>
#include <errno.h>	//perror
#include <fcntl.h>	//open
#include <time.h>	//time
#include <math.h>	//pow

uint8_t* Operation;
uint8_t* compiled_code;

void sharedmem_init(); 		//access shared memory
void sharedmem_exit();
void drecompile_init(); 	// start memory mapping
void drecompile_exit(); 
void* drecompile(uint8_t *func); //optimize

int main(void)
{
	int (*func)(int a);
	int i;
	static struct timespec start_time, end_time;

	sharedmem_init();
	drecompile_init();

	#ifdef RECOMPILE
		func = (int (*)(int a))drecompile(Operation);	//recompile
	#else
		func=(int(*)(int a))compiled_code;	//basic
	#endif
	
	mprotect(compiled_code, PAGE_SIZE, PROT_READ|PROT_EXEC);	//protect change

	clock_gettime(CLOCK_MONOTONIC, &start_time);

	func(2);

	clock_gettime(CLOCK_MONOTONIC, &end_time); //end time

	long time =(end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec);
	printf("%ld nsec\n", time);

	drecompile_exit();
	sharedmem_exit();

	return 0;
}

void sharedmem_init()
{
	int shm_id = shmget(1234, PAGE_SIZE, IPC_CREAT | S_IRUSR | S_IWUSR);	//get shared memory
	if(shm_id==-1){printf("shmget error\n"); return;}		//error

	Operation=(uint8_t*)shmat(shm_id, NULL, 0);	//access shared memory
	if(Operation==(uint8_t*)-1){ printf("shmat error \n"); return;}		//error	
}

void sharedmem_exit()
{
	shmdt(Operation);	//cut shared memory
}

void drecompile_init()
{
	int fd=open("t", O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);	//create file
	write(fd,"hello",5);	//write
	compiled_code=mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);//memory allocate
	if(compiled_code==-1){perror("mmap error"); exit(1);}
	strcpy(compiled_code, Operation);	//copy code
}

void drecompile_exit()
{
	munmap(compiled_code,PAGE_SIZE);
}

void* drecompile(uint8_t* func)
{
	int num=0,save=0,check=0, dl=0;	//num is index, save is optimize operand2 index
	uint8_t op=-1, o1=0, o2=0;	//op= operation, o1=operand1, o2=operand2

	while(1){
		if(Operation[num]==0xC3){compiled_code[save]=Operation[num];break;}
		
		if(Operation[num]==0x83||Operation[num]==0x6b){	//add, sub, imul
			if(Operation[num]==op&&Operation[num+1]==o1){	//duplicated
				if(op==0x83){	//add, sub
					if(o2+Operation[num+2]>=0x80){	//+ -> -
						compiled_code[save++]=o2;//optimize code
						compiled_code[save++]=op;//optimize code
						compiled_code[save++]=o1;//optimize code
						o2=Operation[num+2];
					}
					else{o2+=Operation[num+2];}	//optimize
				}
				else if(op==0x6b){	//imul
					if(o2*Operation[num+2]>=0x80){	//+ -> -
						compiled_code[save++]=o2;//optimize code
						compiled_code[save++]=op;//optimize code
						compiled_code[save++]=o1;//optimize code
						o2=Operation[num+2];
					}
					else{o2*=Operation[num+2];}	//optimize
				}
				num+=3;
			}
			else{	//first new operation
				if(check!=0){	//Optimize previous operations
					if(op==0xf6){	//div
						compiled_code[save++]=op;//optimize code
						compiled_code[save++]=o1;//optimize code
					}
					else{compiled_code[save++]=o2;}
					check=0;o2=0;
				}	
				op=Operation[num];
				o1=Operation[num+1];
				o2=Operation[num+2];
				compiled_code[save++]=Operation[num++];	//optimize code
				compiled_code[save++]=Operation[num++];	//optimize code
				num++;check=1;
			}
		}
		else if(Operation[num]==0xf6){	//div
			if(Operation[num]==op&&Operation[num+1]==o1){	//duplicated
				if(check==1){
					//mov %%ebx %%edx	
					compiled_code[save++]=0x89;//optimize code
					compiled_code[save++]=0xd3;//optimize code
					check++;
				}
				//imul %%edx %%ebx
				compiled_code[save++]=0x0f;//optimize code
				compiled_code[save++]=0xaf;//optimize code
				compiled_code[save++]=0xd3;//optimize code
				num+=2;
			}
			else{	//first new operation
				if(check!=0){//Optimize previous operations
					compiled_code[save++]=o2;//optimize code
					check=0;o2=0;
				}
				op=Operation[num++];//save operation
				o1=Operation[num++];
				o2=1;check=1;	
			}
		}
		else{		//Can't Optimize
			if(check!=0){
				if(op==0xf6){
					compiled_code[save++]=op;//optimize code
					compiled_code[save++]=o1;//optimize code
				}
				else{compiled_code[save++]=o2;}	//optimize code	
				check=0;o2=0;
			}
			op=0;o1=0;
			compiled_code[save++]=Operation[num++];	//copy code
		}
	}
	return compiled_code;
}

