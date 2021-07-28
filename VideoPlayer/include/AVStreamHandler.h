#ifndef __AVSTREAMHANDLER_H__
#define __AVSTREAMHANDLER_H__


#define YES 1
#define NO 0

#include <string>

typedef void (*stream_AnalyticsCB)(std::string);

void * playback_open(const char *filename);

int playback_start(void *);

void registerCallback(stream_AnalyticsCB cb);

void playback_stop(void * ctx);




#endif // __AVSTREAMHANDLER_H__