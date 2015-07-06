#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THER 100000
#define PROCESS_NUM 2

int mapping_table[100000];
int random_pid[100000];
int mapping_index;

void init(void)
{
	int i;
	mapping_index = 0;
	
	for(i = 0; i < 100000; i++) {
		mapping_table[i] = -1;
		random_pid[i] = -1;
	}

	srand(time(NULL));
}

int judge_pid(int blkno)
{
	int i;

	for(i = 0; i < mapping_index; i++) {
		if(abs(blkno - mapping_table[i]) < THER) {
			return random_pid[i];
		}
	}

	mapping_table[mapping_index] = blkno;
	if(mapping_index <= PROCESS_NUM) {
		random_pid[mapping_index] = mapping_index;
	} else {
		random_pid[mapping_index] = rand() % PROCESS_NUM;
	}
	mapping_index++;

	return random_pid[mapping_index-1];
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout;

	int pid;
	double time;
	int devno;
	int blkno;
	int bcount;
	int type;

	int total_char;
	int check_progress_cnt=0;
	int last_progress=0;
	int now_progress;
	

	fin = fopen(argv[1], "r");
	fout = fopen(argv[2], "w");

	fseek(fin,0,2);
	total_char = ftell(fin);
	fseek(fin,0,0);
	

	//init();

	// time = time_stamp
	// devno = device number
	// blkno = lbn
	// bcount = number of 
	while(fscanf(fin, "%lf%d%d%d%d", &time, &devno, &blkno, &bcount, &type) == 5) {
		//pid = judge_pid(blkno);
		fprintf(fout, "%lf %d %d %d %d\n", time/100, devno, blkno, bcount, type);
		if( check_progress_cnt++==100 ){
			now_progress = ftell(fin)*100/total_char;
			if( now_progress != last_progress ){
				for( int i=0; i<now_progress-last_progress; i++ ){
					printf(".");
					fflush(stdout);
				}
				last_progress = now_progress;
			}
			check_progress_cnt=0;
		}
	}
	printf("\n");

	fclose(fin);
	fclose(fout);

	return 0;
}
