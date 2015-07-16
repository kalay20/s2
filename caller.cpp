#include <cassert>
#include <iostream>
#include<queue>

#include <sys/wait.h>
#include <unistd.h>
#include "systemc.h"

//class TestBench;
//extern TestBench* TB_outside_unit;

void Initialize_sc_main(int argc, char* argv[]);
void Wait_Simulation_Done();	
void update();

int main( int argc, char* argv[] )
{
	Initialize_sc_main(argc,argv);

	for(int i=0;i<10000000;i++){
		//usleep(10000);
		if(i==0) printf("The first update!\n");
		//printf("sleep #%d\n",i);
		update();
	}
	Wait_Simulation_Done();	

	return 0;
}

