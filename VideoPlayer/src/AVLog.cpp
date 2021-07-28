/*
 * AVlog.c
 *
 *  Created on: 18-Jul-2021
 *      Author: ankit
 */

#include "AVLog.h"
#include <errno.h>
#include<stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "Analytics.h"
#include <string.h>
extern "C" {
    #include "libavutil/log.h"
}


void *ctx = nullptr;
void attach(void *ctx_arg) {
    ctx = ctx_arg;
}


AVLog* AVLog::avlog_= nullptr;
FILE* AVLog::gFilePtr = nullptr;
pthread_mutex_t AVLog::gMutexLog = PTHREAD_MUTEX_INITIALIZER;

int gffloglevel = AV_LOG_ERROR | AV_LOG_WARNING;

AVLog::AVLog(std::string filename) {
	init_log(filename.c_str());	
}

AVLog * AVLog::GetInstance(std::string filename) {
	if(avlog_ == nullptr) {
		avlog_ = new AVLog(filename);
	}
    return avlog_;
}

AVLog::~AVLog() {
	if (gFilePtr != nullptr) { 
		fclose(gFilePtr);
		pthread_mutex_destroy(&gMutexLog);
	}
}

void AVLog::DestoryInstance() {
	if(avlog_ != nullptr) {
		delete avlog_;
		avlog_ = nullptr;
	}
}

void AVLog::init_log(std::string fname) {

	av_log_set_callback(ffmpeglog);
	av_log_set_level(gffloglevel);
// if (signal(SIGHUP, sighandler) == SIG_ERR)
	//  printf("can't catch SIGHUP\n");
	// if (signal(SIGINT, sighandler) == SIG_ERR)
	//  printf("can't catch SIGINT\n");
	if (signal(SIGQUIT, sighandler) == SIG_ERR)
		printf("can't catch SIGQUIT\n");
	if (signal(SIGTERM, sighandler) == SIG_ERR)
		printf("can't catch SIGTERM\n");
	//   if (signal(SIGKILL, sighandler) == SIG_ERR)
	//    printf("can't catch SIGKILL\n");
	if (signal(SIGFPE, sighandler) == SIG_ERR)
		printf("can't catch SIGFPE\n");
	if (signal(SIGILL, sighandler) == SIG_ERR)
		printf("can't catch SIGILL\n");
	if (signal(SIGSEGV, sighandler) == SIG_ERR)
		printf("can't catch SIGSEGV\n");
	if (signal(SIGBUS, sighandler) == SIG_ERR)
		printf("can't catch SIGBUS\n");
	if (signal(SIGABRT, sighandler) == SIG_ERR)
		printf("can't catch SIGABRT\n");

	gFilePtr = fopen(fname.c_str(),"wt");
	if (gFilePtr == NULL) {
		printf("Error:Unable to open Log File:%s\n",fname.c_str());
		return;
	}
}

void AVLog::ffmpeglog(void* p1, int p2, const char* pFormat, va_list args) {
	
	if(gffloglevel >= p2) {
		char buffer[10000];
		vsprintf(buffer, pFormat, args);
		
		if(ctx != nullptr) {	
			((Analytics *)ctx)->add_error(buffer);
		}

#ifdef DEBUG_LOG
		pthread_mutex_lock(&gMutexLog);
		char buff[100];
		time_t now = time (0);
		strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));
		fprintf(gFilePtr,"[%s] FFMPEG => %s",buff, buffer);
		fflush(gFilePtr);
		pthread_mutex_unlock(&gMutexLog);
#endif
	}
}
/*Function Name: AVLog::log_to_file
 *Input Arguments: list of inout arguments
 *Output Arguments: void
 *Description: This function logs the received message in the file
 */
void AVLog::log_to_file(const char *pFormat,...)
{
#ifdef DEBUG_LOG
	va_list args;
	pthread_mutex_lock(&gMutexLog);
	va_start(args, pFormat);
	char buff[100];
	time_t now = time (0);
	strftime (buff, 100, "%Y-%m-%d %H:%M:%S.000", localtime (&now));
	fprintf(gFilePtr,"[%s] SIMULATOR => ",buff);
	vfprintf(gFilePtr,pFormat, args);
	fprintf(gFilePtr,"\n");
	va_end(args);
	fflush(gFilePtr);
	pthread_mutex_unlock(&gMutexLog);
#endif
}

void AVLog::sighandler(int sig)
{
	if(gFilePtr) {
		log_to_file("Error: caught signal %d",sig);
		fflush(gFilePtr);
	}
	printf("Error: caught signal %d\n",sig);
	// call default handler
	signal(sig, SIG_DFL);
	raise(sig);
	//restore out handler
	//if (signal(sig, sighandler) == SIG_ERR)
	// printf("can't catch %d\n",sig);
}
