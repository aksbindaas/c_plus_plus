#ifndef __PLAYERMANAGER_H__
#define __PLAYERMANAGER_H__

#include <string>

typedef  void (*analytics_callback)(std::string);

struct PlaybackContext {
    void * streamCtx;
};

PlaybackContext * load(char *stream_url, char *log_file);

void start(PlaybackContext *ctx);

void stop(PlaybackContext *ctx);

void register_analyticsCB(analytics_callback analyticsCB);


#endif // __PLAYERMANAGER_H__