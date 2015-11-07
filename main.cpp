#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// LaiYang
#include <pthread.h>
#include <semaphore.h>
// end LaiYang

#include "systemc.h"
#include "gyc_bus.h"

#include "ssd_typedef.h"
#include "simple_io_scheduler.h"
#include "simple_ssd_controller.h"

using namespace std;
using namespace sc_core;


// LaiYang
bool all_req_completed=false;
// end LaiYang


//#define CYCLE_DRIVEN

#ifdef CYCLE_DRIVEN
// LaiYang
sem_t mutex,m2,mb;
void* f(void*);

int cnt(100);
int caller_over(0);
int loopcnt(0);


void update()
{
	sem_post(&m2);
}

void* interface_sc_main(void* arg)
{
	int retVal;
	int argc=*((int**)arg)[0];
	char **argv = (char**)(((int**)arg)[1]);

	printf("argc=%d\n",argc);
	for( int i=0; i<argc; i++ ) 	printf(" %s",argv[i] );
	printf("\n");

	retVal = sc_main(argc,argv);
	if( retVal<0 ){
		printf("sc_main() return<0.\n");
	}

	return NULL;
}

void Initialize_sc_main(int argc, char* argv[] )
{
	int retVal;
	pthread_t tid;

	if( sem_init(&mutex,1,0)<0 ){
		perror("mutex initial fail.\n");
		exit(-1);
	}
	if( sem_init(&m2,1,0)<0 ){
		perror("mutex initial fail.\n");
		exit(-1);
	}
	if( sem_init(&mb,1,0)<0 ){
		perror("mutex initial fail.\n");
		exit(-1);
	}

	void* arg = malloc(sizeof(int*)*2);
	((int**)arg)[0] = &argc;
	((int**)arg)[1] = (int*)argv;

	
	retVal = pthread_create( &tid, NULL, interface_sc_main, arg );
	assert(retVal==0);

	sem_wait(&mutex);
}

void Wait_Simulation_Done()
{
	caller_over=1;
	sem_post(&m2);
	printf("wait simualtion done.\n");
	sem_wait(&mutex);
}

SC_MODULE(Initialize_Done){
public:
	void signal()
	{
		printf("Init Done, Simulation started.\n");
		sem_post(&mutex);
	}

	SC_CTOR(Initialize_Done){
		SC_THREAD(signal);
	}
};




// end LaiYang


int sc_main(int argc, char* argv[]) {

	// LaiYang
	Initialize_Done ID("ID");
	
	printf("%d\n",argc);
	for( int i=0; i<argc; i++ ){
		printf("%s\n",argv[i]);
	}

	// end LaiYang

	/* fixed random seed for debug */
	srand(308411);

	/* global memory pool */
	gyc_bus_pkt_mm* global_mm = new gyc_bus_pkt_mm();

	/*if (argc != 3 && argc != 4) {
		cout << "usage 1: " << argv[0] << " [trace path] [disk index]" << endl;
		cout << "usage 2: " << argv[0] << " [trace path] [disk index] [min die num]" << endl;
		exit(-1);
	}*/

	cout << "===================== SIMPLE DIE FTL 20140703 v1 ========================" << endl;
	string trace_path;
	int disk_index;
	int min_die_num = 1;
	double last = -1; // -1 for nothing; 0 for load; other for save
	trace_path = argv[1];
	disk_index = atoi(argv[2]);
	if (argc >= 4) {
		if(strcmp(argv[3], "-l") == 0){
			last = 0;
		}
		else if(strcmp(argv[3], "-s") == 0){
			if(argc < 5){
				cout << "err: " << " [save to which line] " << endl;
				exit(-1);
			}
			last = atof(argv[4]);
		}
		else min_die_num = atoi(argv[3]);
	}
	simple_io_scheduler host("io_sched", global_mm, trace_path, disk_index, last);

	/* simulation setting */
	int number_of_dice = max(host.get_minimum_number_of_dice_required(), min_die_num);
	int number_of_channels = min(SSD_CHANNEL_NUMBER, number_of_dice);
	cout << "number of dice = " << number_of_dice << endl;
	cout << "number of channels = " << number_of_channels << endl;
	int min_free_blocks_percentage = 5;
	slc_policy_t slc_policy = NO_SLC_POLICY;

	simple_ssd_controller ssd(
			"ssd",
			global_mm,
			number_of_dice,
			number_of_channels,
			min_free_blocks_percentage,
			slc_policy,
			last,
			trace_path);

	gyc_bus download_bus("download_bus");
	download_bus.set_bus_id(0);
	host.set_download_ch_id(0);
	ssd.set_download_ch_id(0);
	host.download_ch_port(download_bus);
	ssd.download_ch_port(download_bus);
	download_bus.dev(host);
	download_bus.dev(ssd);

	gyc_bus upload_bus("upload_bus");
	upload_bus.set_bus_id(1);
	host.set_upload_ch_id(1);
	ssd.set_upload_ch_id(1);
	host.upload_ch_port(upload_bus);
	ssd.upload_ch_port(upload_bus);
	upload_bus.dev(host);
	upload_bus.dev(ssd);

	if(last == 0) ssd.load_last_use(trace_path, number_of_dice);

	
	sc_start(SC_ZERO_TIME);
	while( !caller_over ){
		sc_start(sc_time(5,SC_US));
		sem_wait(&m2);
	}
	sc_start();
	
	
	// LaiYang
	assert( all_req_completed );
    cout << "Total execution time: " << sc_time_stamp() << endl;
    // end LaiYang

	ssd.debug_check_all_req_completed();
	host.print_score();
	
	//if(last == 0)ssd.save_last_use(trace_path+"c", last, number_of_dice);
	//if(last == -1)ssd.save_last_use(trace_path+"f2", last, number_of_dice);
	/* check memory leakage */
	/* if there're any memory leakage, then your code may have other bugs (e.g. un-finished request) */
	assert(global_mm->count() == 0);

	
	

	printf("Sim done.\n");
	sem_post(&mutex);

	return 0;
}

#else



int sc_main(int argc, char* argv[]) {

	/* fixed random seed for debug */
	srand(308411);

	/* global memory pool */
	gyc_bus_pkt_mm* global_mm = new gyc_bus_pkt_mm();

	/*if (argc != 3 && argc != 4) {
		cout << "usage 1: " << argv[0] << " [trace path] [disk index]" << endl;
		cout << "usage 2: " << argv[0] << " [trace path] [disk index] [min die num]" << endl;
		exit(-1);
	}*/

	cout << "===================== SIMPLE DIE FTL 20140703 v1 ========================" << endl;
	string trace_path;
	int disk_index;
	int min_die_num = 1;
	double last = -1; // -1 for nothing; 0 for load; other for save
	trace_path = argv[1];
	disk_index = atoi(argv[2]);
	if (argc >= 4) {
		if(strcmp(argv[3], "-l") == 0){
			last = 0;
		}
		else if(strcmp(argv[3], "-s") == 0){
			if(argc < 5){
				cout << "err: " << " [save to which line] " << endl;
				exit(-1);
			}
			last = atof(argv[4]);
		}
		else min_die_num = atoi(argv[3]);
	}
	simple_io_scheduler host("io_sched", global_mm, trace_path, disk_index, last);

	/* simulation setting */
	int number_of_dice = max(host.get_minimum_number_of_dice_required(), min_die_num);
	int number_of_channels = min(SSD_CHANNEL_NUMBER, number_of_dice);
	cout << "number of dice = " << number_of_dice << endl;
	cout << "number of channels = " << number_of_channels << endl;
	int min_free_blocks_percentage = 5;
	slc_policy_t slc_policy = NO_SLC_POLICY;

	simple_ssd_controller ssd(
			"ssd",
			global_mm,
			number_of_dice,
			number_of_channels,
			min_free_blocks_percentage,
			slc_policy,
			last,
			trace_path);

	gyc_bus download_bus("download_bus");
	download_bus.set_bus_id(0);
	host.set_download_ch_id(0);
	ssd.set_download_ch_id(0);
	host.download_ch_port(download_bus);
	ssd.download_ch_port(download_bus);
	download_bus.dev(host);
	download_bus.dev(ssd);

	gyc_bus upload_bus("upload_bus");
	upload_bus.set_bus_id(1);
	host.set_upload_ch_id(1);
	ssd.set_upload_ch_id(1);
	host.upload_ch_port(upload_bus);
	ssd.upload_ch_port(upload_bus);
	upload_bus.dev(host);
	upload_bus.dev(ssd);

	if(last == 0) ssd.load_last_use(trace_path, number_of_dice);

	sc_start();
	
	// LaiYang
	assert( all_req_completed );
    	cout << "Total execution time: " << sc_time_stamp() << endl;
    	// end LaiYang

	ssd.debug_check_all_req_completed();
	host.print_score();
	
	//if(last == 0)ssd.save_last_use(trace_path+"c", last, number_of_dice);
	//if(last == -1)ssd.save_last_use(trace_path+"f2", last, number_of_dice);
	/* check memory leakage */
	/* if there're any memory leakage, then your code may have other bugs (e.g. un-finished request) */
	assert(global_mm->count() == 0);

	return 0;
}

#endif




