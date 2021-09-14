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

#define USAGE	"usage: marchive host"

const int chnum = 19;
chid		chan[chnum];
struct      dbr_ctrl_float ch_fild[chnum];
float       samplevalue[chnum];
float       statusvalue[chnum];
char* chname[chnum]   = {"acu_power_1", "acu_power_2", "acu_power_3", "acu_power_4", "acu_power_5",
                         "acu_src_pos_1", "acu_src_pos_2", "acu_src_pos_3", "acu_src_pos_4", "acu_src_pos_5",
                         "acu_mot_I_1", "acu_mot_I_2", "acu_mot_I_3", "acu_mot_I_4", "acu_mot_I_5",
                         "acu_mot_load_1", "acu_mot_load_2", "acu_mot_load_3", "acu_mot_load_4"}; 

char* archchname[chnum]={"power_1", "power_2", "power_3", "power_4", "power_5",
                         "src_pos_1", "src_pos_2", "src_pos_3", "src_pos_4", "src_pos_5",
                         "mot_I_1", "mot_I_2", "mot_I_3", "mot_I_4", "mot_I_5",
                         "mot_load_1", "mot_load_2", "mot_load_3", "mot_load_4"}; 
char* sinchname = "sinch";
chid  sinchan;

void listen_event(struct event_handler_args args);
void listen_sinevent(struct event_handler_args args);
void save_sample(MYSQL* database);
void save_status(MYSQL* database, int evokech);

int main(int argc, char **argv)
{
	unsigned	i;
	char		*host;
	float		pvalue;
	int		status;
    char*     runmode;

	/*
 	 *	Check for valid arguments
	 */
	if(argc<1){
		printf("%s\n",USAGE);
		exit(-1);
	}
	host = argv[1];
    runmode = argv[2];


	/*
	 *	Initialize CA
	 */
  	SEVCHK(ca_task_initialize(),"Unable to initialize");


	/*
	 *	get the channel id
	 */
    
    for(int i=0;i<chnum;i++) {
  	    status = ca_search(	chname[i], &chan[i]);
  	    SEVCHK(status, "Bad Channel Name?")
    }
    status = ca_search(sinchname, &sinchan);
    SEVCHK(status, "Bad Channel Name?")


	/*
	 *	wait for the channel's IOC to be found
	 */
	status = ca_pend_io(1.0);
	if(status == ECA_TIMEOUT){
		printf("%s: Not Found\n", chname[0]);
		exit(-1);
	}
  	SEVCHK(status, NULL)

     
    status = ca_pend_io(1.0);
    SEVCHK(status, NULL)

	/*
     *  setup MySQL database
     */
	MYSQL *mysql_init(MYSQL *mysql);
    MYSQL *MySQLConRet;
    MYSQL *conn;
    conn = mysql_init(NULL);
    MySQLConRet = mysql_real_connect(conn, host, "huanggh", "123456", "acucalibsys", 3306, NULL, 0);
    if (MySQLConRet == NULL){
        printf("ERROR\n");
    }

     
    int save_signal = -1;
	for(int i=0;i<5;i++) {
        status = ca_create_subscription(DBR_CTRL_INT,
               1, chan[i], DBE_VALUE, listen_event, &save_signal, NULL);
        SEVCHK(status, NULL);
    }
	for(int i=5;i<chnum;i++) {
        status = ca_create_subscription(DBR_CTRL_FLOAT,
               1, chan[i], DBE_ALARM, listen_event, &save_signal, NULL);
        SEVCHK(status, NULL);
    }
    status = ca_pend_event(1.0);
    save_signal = -1;
    

    status = ca_create_subscription(DBR_FLOAT,
               1, sinchan,  DBE_VALUE, listen_sinevent, conn, NULL);
    SEVCHK(status, NULL);

      
    struct timeval start, end;  // record time consumption

    int sampcount = 0;
    gettimeofday(&start, NULL); 

    //estimate time consumption of writing sql (~0.3 ms)
    if(!strncmp(runmode, "0", 1)) {
        printf("mode0 \n");
        while(sampcount<1E4) {
           	for(int i=0;i<chnum;i++) {
                statusvalue[i] = 1;
            	samplevalue[i] = 0.5;
        	}
            save_sample(conn);
            sampcount ++;
        }

    }
    //estimate time consumption of ca_get (~55 us)
    else if(!strncmp(runmode, "1", 1)) {
        printf("mode1 \n");
        while(sampcount<1E5) {
            for(int i=0;i<chnum;i++) {
                status = ca_get(DBR_CTRL_FLOAT, chan[i], &ch_fild[i]);
                SEVCHK(status,NULL);
            }
            status = ca_pend_io(1);
            SEVCHK(status,NULL);

            sampcount ++;
        }
    }
    //estimate time consumption of trigger event (~ ) 
    else if (!strncmp(runmode, "2", 1)) {
        printf("mode2 \n");
        status = ca_pend_event(10);   
        SEVCHK(status, "time out");
    }

    gettimeofday(&end, NULL);


	double time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    if(!strncmp(runmode, "0", 1)) time /= 1.e4;
    if(!strncmp(runmode, "1", 1)) time /= 1.e5;
    if(!strncmp(runmode, "2", 1)) time /= 1.;

    printf("runmode: %s, time: %g ms \n", runmode, time/1000);
    //printf("triggerNum: %g \n", triggerNum);

	return 0;
}


void save_sample(MYSQL* database)
{

    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    char query_sample[2000];
    sprintf(query_sample, "INSERT INTO sample (smpl_time, %s, %s, %s, %s, %s,   %s, %s, %s, %s, %s,   %s, %s, %s, %s) \ 
            VALUES ('%s', %g, %g, %g, %g, %g,   %g, %g, %g, %g, %g,    %g, %g, %g, %g)",  
            archchname[5], archchname[6], archchname[7], archchname[8], archchname[9], 
            archchname[10], archchname[11], archchname[12], archchname[13], archchname[14],
            archchname[15], archchname[16], archchname[17], archchname[18], buffer, 

              samplevalue[5],   samplevalue[6],   samplevalue[7],   samplevalue[8], samplevalue[9],
              samplevalue[10],   samplevalue[11],   samplevalue[12],   samplevalue[13], samplevalue[14],
              samplevalue[15],   samplevalue[16],   samplevalue[17],   samplevalue[18]);

    //printf("%s", query_sample) ;

    int sqlstat = mysql_query(database, query_sample);
    if(sqlstat) printf("%d", sqlstat);
}


void listen_event(struct event_handler_args args)
{

    char evokechname[200];
    sprintf(evokechname, "%s", ca_name(args.chid));
    for(int i=0;i<5;i++) {
        char acuid[10];
        sprintf(acuid, "_%d", i+1);
        if(strstr(evokechname, acuid)) {
            *(int*)args.usr = i;
            break;
        }
    }
    if(strstr(evokechname, "power")) {
        printf("%s: %s\n", ca_name(args.chid), (*(struct dbr_ctrl_int *)args.dbr).value == 1? "ON":"OFF");
    } else {
        printf("%s: %s\n", ca_name(args.chid), alarmStatusString[(*(struct dbr_ctrl_float *)args.dbr).status]);
    }

}

void listen_sinevent(struct event_handler_args args)
{
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    char query_sample[2000];
    sprintf(query_sample, "INSERT INTO sinsample (smpl_time, value) VALUES ('%s', %g)", buffer, *(float*)args.dbr);

    int sqlstat = mysql_query((MYSQL*)args.usr, query_sample);
    if(sqlstat) printf("%d", sqlstat);
}



void save_status(MYSQL* database, int evokech)
{

    int status;
    
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    int power_status = 0;
    int pos_status = 0;
    int current_status = 0;
    int load_status = 0.;
    power_status = samplevalue[evokech];
    pos_status = statusvalue[evokech+5];
    current_status = statusvalue[evokech+10];
    if(evokech<4) load_status = statusvalue[evokech+15];

    char query_status[2000];
    sprintf(query_status, "INSERT INTO samplestatus (acu_id, smpl_time, power_status, pos_status, current_status, load_status) VALUES ('%d', '%s', %d, %d, %d, %d)", 
          evokech+1, buffer,  power_status, pos_status, current_status, load_status);


    int sqlstat = mysql_query(database, query_status);
    if(sqlstat) printf("%d", sqlstat);

}

