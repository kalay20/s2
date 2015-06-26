#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sys/wait.h>


#include "systemc.h"

using namespace std;
using namespace sc_core;


// LaiYang
sem_t mutex,m2,mb;
void* f(void*);

int cnt(100);
int caller_over(0);
int loopcnt(0);

SC_MODULE(epoch_profiler){
public:
	sc_event na,nb;

	void update()
	{
		/*while(true){
			sem_wait(&m2);
			wait(sc_time(5,SC_US));
			//wait(sc_time(100,SC_SEC));
		}*/
	}

	
	void B()
	{
		if( cnt-->0 )
			nb.notify(sc_time(20,SC_US));
		if( cnt%10==9 ){
			cout << sc_time_stamp();
			printf(",%d\n",cnt);
		}
	}

	SC_CTOR(lock_Clock){
		//SC_METHOD(update);
		//sensitive << clk.pos();
		//dont_initialize();
		SC_METHOD(B);
		sensitive << nb;
		//dont_initialize();
	}
};

void update()
{
	sem_post(&m2);
	sem_wait(&mb);
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
		perror("mutex initial fall.\n");
		exit(-1);
	}
	if( sem_init(&m2,1,0)<0 ){
		perror("mutex initial fall.\n");
		exit(-1);
	}
	if( sem_init(&mb,1,0)<0 ){
		perror("mutex initial fall.\n");
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
	int kl=0;
	// LaiYang
	Initialize_Done ID("ID");
	lock_Clock c1("c1");
	//sc_clock clk("tc",5,SC_US);

	sc_report_handler::set_actions (SC_WARNING, SC_DO_NOTHING);
	
	// end LaiYang
	//scstart(sc_time(0,SC_US));
	sc_start(SC_ZERO_TIME);
	sem_wait(&m2);
	while( !caller_over ){
		if( ++kl%10==0 ) printf("kl=%d\n",kl);

		sc_start(sc_time(5,SC_US));
		sem_post(&mb);
		sem_wait(&m2);
	}
	sc_start();
	//sc_start(sc_time(1000,SC_US));
	//sc_start();

	// LaiYang
	cout << "Total execution time: " << sc_time_stamp() << endl;
	// end LaiYang
	//sc_pending
	

	printf("Sim done.\n");
	sem_post(&mutex);

	return 0;
}

