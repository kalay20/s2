

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

const int el_start=0;
const int el_end=5;
const int wl_start=0;
const int wl_end=5;
const int dn_start=0;
const int dn_end=5;
const int bs_start=0;
const int bs_end=4;
const int ch_start=0;
const int ch_end=5;

//int wl[7] = {1,2,4,8,16,32,64};
//int dn[6] = {20,40,80,160,320,640};

int el[]={ 100, 150, 200, 250, 300 };
int	wl[]={ 100, 150, 200, 250, 300 };
int	dn[]={ 64, 80, 96, 112, 128 };
int	bs[]={ 128, 256, 512, 1024 };
int ch[]={ 1, 2, 4, 8, 16 };



const int workload_size = 6;
char workload_short_ori[workload_size][10] = {"mse","msn","msl","usr1","src10","synf"};
char workload_short[workload_size][10];

// ---------------------- change ------------------------------------
int workload_bool[] = {1,1,1,1,1,1};
const int change=1;
const int burst=0;

// ---------------------- change function ------------------------------------
int p1_start, p2_start;
int p1_end, p2_end;
int *p1, *p2;

void Change_init()
{
	for( int i=0; i<workload_size; i++ ){
		if( burst==0 ){
			sprintf( workload_short[i], "%s", workload_short_ori[i] );
		}
		else if( burst==1 ){
			sprintf( workload_short[i], "%s%s", workload_short_ori[i], "_b" );
		}
	}

	if( change==1 ){
		p1_start=wl_start; p1_end=wl_end; p1=wl;
		p2_start=dn_start; p2_end=dn_end; p2=dn;
	}
	else if( change==4 ){
		p1_start=el_start; p1_end=el_end; p1=el;
		p2_start=dn_start; p2_end=dn_end; p2=dn;
	}
	else if( change==5 ){
		p1_start=bs_start; p1_end=bs_end; p1=bs;
		p2_start=dn_start; p2_end=dn_end; p2=dn;
	}
}
// ---------------------- end change ------------------------------------

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

int SD_unit( const char* s, const char* ss, double& d, char* unit )
{
	s = strstr(s,ss);
	if( s==NULL ){
		printf("Word not found: %s\n",ss);
		return -1;//H_error("No match word found !!");
	}

	s += strlen(ss);
	sscanf( s, "%lf%s", &d, unit  );

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

	Change_init();

	ofp = fopen("out.csv","a");
	if( ofp==NULL ) H_error("ofp open fail");

	
	for( int wd=0; wd<workload_size; wd++ ){
		if( workload_bool[wd] == 0 ) continue;

		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double pr,pw,t;
				int ret=0;
				char unit[100];

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "User Page Read: ", pr );
				ret |= SD( buf, "User Page Write: ", pw );
				ret |= SD_unit( buf, "Total execution time: ", t, unit );

				if( strcmp(unit,"ns")==0 ){
					t /= (double)1000.0*1000.0*1000.0;
				}
				else if( strcmp(unit,"us")==0 ){
					t /= (double)1000.0*1000.0;
				}
				else if( strcmp(unit,"ms")==0 ){
					t /= (double)1000.0;
				}
				else if( strcmp(unit,"s")==0 ){
					t /= (double)1;
				}


				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", (pr+pw)/t );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}
		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double iops,pr,pw,t;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "Peak IOPS: ", iops );


				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", iops );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}

		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double latency;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "average I/O latency = ", latency );

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", latency );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}

		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double gc_count;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "GC Block Erase: ", gc_count );

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", gc_count );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}

		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double wa;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
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

		for( i=p1_start; i<p1_end; i++ ){
			for(  j=p2_start; j<p2_end; j++ ){
				char fn[100];
				double gc_count, live_page_copy, block_size;
				int ret=0;

				sprintf( fn, "%s_%d_%d", workload_short[wd], p1[i], p2[j] );
				//printf("%s\n",fn);
				ifp = fopen(fn,"r");
				if( ifp==NULL ) H_error(fn);

				RF(ifp,buf);
				fclose(ifp);

				ret |= SD( buf, "# of GC Page Write: ", live_page_copy );
				ret |= SD( buf, "# of GC Block Erase: ", gc_count );

				if( change==5 ){
					block_size=p1[i];
				}
				else{
					block_size=128;
				}

				if( ret != 0 ){
					fprintf( ofp, "%f,", 0 );
					continue;
				}

				fprintf( ofp, "%f,", live_page_copy / gc_count / block_size );
				//printf("%s",buf);
			}
			fprintf(ofp,"\n");
		}
	}



	fclose(ofp);
	return 0;
}





