#include "PlayerManager.h"
#include "AVStreamHandler.h"
#include "AVLog.h"

#include <unistd.h>

PlaybackContext * load(char *stream_url,char *log_file) {
    if (AVLog::GetInstance(log_file) == NULL) {
		printf("Error:Unable to open Log File:%s\n",log_file);
		playback_stop(nullptr);
		return nullptr;
	}
	
	PlaybackContext *ctx = new PlaybackContext;
	AVLog::log_to_file("Starting CS version %s  PID %d with parameters(%s,%s)",CSVERSION, getpid(), stream_url, log_file);
	ctx->streamCtx =  playback_open(stream_url);
	return ctx;
}

void start(PlaybackContext *ctx) {
	playback_start(ctx->streamCtx);
}

void stop(PlaybackContext *ctx) {
    AVLog::log_to_file("All threads deleted Main Thread Exiting..processid[%d]",getpid());
	AVLog::DestoryInstance();
	if(ctx != nullptr) {
		playback_stop(ctx->streamCtx);
		delete ctx;
	}
}

void register_analyticsCB(analytics_callback cb) {
	registerCallback(cb);
}