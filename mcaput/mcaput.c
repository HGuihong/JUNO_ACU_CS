/*************************************************************************\
\*************************************************************************/
/*
 *
 *	Guihong Huang
 *
 *	Test OPI CA client 
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mysql.h>
#include <cadef.h>
#include <db_access.h>
#include "alarmString.h"


const int chnum = 19;
chid		chan[chnum];
struct      dbr_ctrl_float ch_fild[chnum];
float       samplevalue[chnum];
float       statusvalue[chnum];

char* ttestchname = "ttestch";
chid  ttestchan;

int main(int argc, char **argv)
{
	unsigned	i;
	char		*host;
	float		pvalue;
	int		status;

	/*
	 *	Initialize CA
	 */
  	SEVCHK(ca_task_initialize(),"Unable to initialize");


	/*
	 *	get the channel id
	 */
    
    status = ca_search(ttestchname, &ttestchan);
    SEVCHK(status, "Bad Channel Name?")


	/*
	 *	wait for the channel's IOC to be found
	 */
	status = ca_pend_io(1.0);
	if(status == ECA_TIMEOUT){
		printf("%s: Not Found\n", "##");
		exit(-1);
	}
  	SEVCHK(status, NULL)

     
    status = ca_pend_io(1.0);
    SEVCHK(status, NULL)

    
    struct timeval start, end;
    float temp = 0.;
    for(int i=0;i<5E2;i++) {
        gettimeofday(&start, NULL);
        temp += 1.;
        status = ca_put(DBR_FLOAT, ttestchan, &temp);
        SEVCHK(status, NULL)

        status = ca_flush_io();
        SEVCHK(status, NULL)

        gettimeofday(&end, NULL);
        double begintime = start.tv_sec * 1000000. +  start.tv_usec;
        double endtime = end.tv_sec * 1000000. +  end.tv_usec;
        double time = endtime - begintime;
        printf("%d   %.f  %.f \n", i, begintime, time);
        sleep(1);

    }
      
	return 0;
}
