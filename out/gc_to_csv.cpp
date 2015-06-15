

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

const int el_end=5;
const int wl_end=5;
const int dn_end=5;
const int bs_end=4;

//int wl[7] = {1,2,4,8,16,32,64};
//int dn[6] = {20,40,80,160,320,640};

int el[el_end]={ 100, 150, 200, 250, 300 };
int	wl[wl_end]={ 100, 150, 200, 250, 300 };
int	dn[dn_end]={ 64, 80, 96, 112, 128 };
int	bs[bs_end]={ 128, 256, 512, 1024 };



int workload_size = 6;
char workload_short[][10] = {"mse","msn","msl","usr1","src10","synf"};
int workload_bool[] = {1,1,1,1,1,1};

void H_error( const char* _Msgstderr )
{
	fprintf( stderr, "%s\n\n", _Msgstderr );
	exit(-1);
}

int SD( const char* s, const char* ss, double& d )
{
	s = strstr(s,ss);
	if( s==NULL ){
		printf("Word not found: %s\n",ss);
		return -1;//H_error("No match word found !!");
	}

	s += strlen(ss);
	sscanf( s, "%lf", &d  );

	//printf("%s\nHL=%f",s,d);

	return 0;	
}

int RF( FILE* fp, char* s )
{
	int cnt=0;
	while( (*s=getc(fp)) != EOF ){
		s++;
		cnt++;
	}
	*s='\0';
	return cnt;
}

int main( int argc, char* argv[] )
{
	FILE* ifp, *ofp;
	int i,j;
	char buf[10000];

	ofp = fopen("out.csv","a");
	if( ofp==NULL ) H_error("ofp open fail");

	
	for( int wd=0; wd<workload_size; wd++ ){
		if( workload_bool[wd] == 0 ) continue;
		
		for( i=0; i<bs_end; i++ ){
			for(  j=0; j<dn_end; j++ ){
				char fn[100];
				double gc_cnt,wa,perc,live_per_gc;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], bs[i], dn[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "GC Block Erase: ", gc_cnt );

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", gc_cnt );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}

		for( i=0; i<bs_end; i++ ){
			for(  j=0; j<dn_end; j++ ){
				char fn[100];
				double gc_cnt,wa,perc,live_per_gc;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], bs[i], dn[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "Write Amplification: ", wa );

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", wa );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}
		for( i=0; i<bs_end; i++ ){
			for(  j=0; j<dn_end; j++ ){
				char fn[100];
				double gc_cnt,wa,perc,live_per_gc;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], bs[i], dn[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "avg live page copy per gc: ", live_per_gc );

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", live_per_gc/bs[i] );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}
	}


	fclose(ofp);
	return 0;
}





