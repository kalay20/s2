#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[] )
{
	FILE* ifp;
	double d;
	double sum=0;
	int line_cnt=0;

	if( argc != 3 ) exit(-2);
	//printf("%d\n",atoi(argv[1] ));

	ifp = fopen("cpu.dat","r");
	if( ifp==NULL ) exit(-2);


	while( fscanf(ifp,"%lf",&d)==1 ) {
		sum+=d;
		line_cnt++;
		//printf("%f\n",d);
	}
	
	fclose(ifp);
	if( atoi(argv[2])==1 ){
		if( sum>=atoi(argv[1]) ) return -1;
	}
	else if( atoi(argv[2])==2 ){
		if( line_cnt>=atoi(argv[1]) ) return -1;
	}
	else
		exit(-2);

	return 0;
}




