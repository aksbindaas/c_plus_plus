#include "AVStreamHandler.h"

#include <unistd.h>
#include "AVLog.h"
#include <string>
#include <iostream>
#include "Analytics.h"

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/time.h"
}

class VideoState {

public:
	AVFormatContext *formateContext;
	AVProgram *program;
    AVStream* video_st;
    AVStream* audio_st;

	int videoStream;	
	double startTimeOffset;
	double currentPosition;
    int64_t vBuffer;

	int audioStream;
	int64_t audio_pts;
	double audio_clock;
	int64_t aBuffer;
    
    VideoState () {
        formateContext = nullptr;
		program = nullptr;
        video_st = nullptr;
        audio_st = nullptr;
        videoStream = -1;
        startTimeOffset = -1.0;
        currentPosition = 0.0;
        vBuffer = 0;
        
        audioStream = -1;
        audio_pts = 0;
        audio_clock = 0;
        aBuffer = 0;
    }
};
class StreamInfo {
public:
    int nStreamId;
    int bReadyToPlay;
    int bPause;
    int bDeleteStream;
    int bStreamError;
    char splayerId[37];
    std::string filename;
    VideoState videoState;
	Analytics analytics_ctx;
    double starttime;

    StreamInfo() {
        nStreamId = 0;
        bReadyToPlay = YES;
        bPause = NO;
        bDeleteStream = NO;
        bStreamError = NO;
        starttime = 0.0;
    }
};

stream_AnalyticsCB callback = nullptr;

void registerCallback(stream_AnalyticsCB cb) {
	callback = cb;
}

void analytics_log(StreamInfo *sinfo) {
	sinfo->analytics_ctx.add_to_json("stream_url",sinfo->filename);
	sinfo->analytics_ctx.add_to_json<int>("stream_id", sinfo->nStreamId);
	sinfo->analytics_ctx.add_to_json<double>("startTimeOffset",sinfo->videoState.startTimeOffset/1e+6);
	sinfo->analytics_ctx.add_to_json<double>("currentPosition",sinfo->videoState.currentPosition);

	if(callback != nullptr) {
		callback(sinfo->analytics_ctx.to_string());
	} else {
		AVLog::log_to_file("%s",sinfo->analytics_ctx.to_string().c_str());
	}
}

// int ff_interupt(void* s) {
//     StreamInfo *streaminfo = (StreamInfo*) s;
// 	AVLog::log_to_file("interupting ffmpeg for stream %d as stream marked for deletion",streaminfo->nStreamId);
//     return 0;
// }

int openstream(AVFormatContext **fctx,StreamInfo *sinfo) {
    
	*fctx = avformat_alloc_context();
	//AVIOInterruptCB icallback =  {ff_interupt,sinfo};
    //(*fctx)->interrupt_callback = icallback;
	
	attach(&sinfo->analytics_ctx);

    snprintf(sinfo->splayerId,sizeof(sinfo->splayerId),"ID%.3dIDX%.3dTM-%" PRIu64 "",sinfo->nStreamId, 0, av_gettime());
	char header[100];
	snprintf(header,sizeof(header),"X-Playback-Session-Id: %s\r\n",sinfo->splayerId);

	AVDictionary *opts = NULL;

	av_dict_set(&opts, "headers", header, 0);      // add an entry
	int ret = avformat_open_input(fctx,sinfo->filename.c_str(), NULL, &opts);
	av_dict_free(&opts);
	if (ret != 0) {
		analytics_log(sinfo);
        AVLog::log_to_file("Failed to open context %s",sinfo->filename.c_str());
		return -1;
	}

	if (*fctx != NULL && (ret = avformat_find_stream_info(*fctx, NULL)) < 0) {
		AVLog::log_to_file("av_find_stream_info(%s) Err:[%d]]",sinfo->filename.c_str(),ret);
		ret = -1;
	}


	if(ret >= 0) {
		AVLog::log_to_file("Successfully opened Stream [%s]",sinfo->filename.c_str());
		ret = 0;
		sinfo->starttime = av_gettime();
		sinfo->videoState.formateContext = *fctx;
		sinfo->videoState.startTimeOffset = (*fctx)->start_time;
		sinfo->videoState.program = (*fctx)->programs[0];

	} else {
		if(*fctx) avformat_free_context(*fctx);
		AVLog::log_to_file("Failed to open stream [%s]-[%d]",sinfo->filename.c_str(),ret);
	}
	return ret;
}


int stream_component_open(StreamInfo * sInfo, int stream_index) {

	VideoState *videoState = &sInfo->videoState;
	AVFormatContext *pFormatCtx = videoState->formateContext;

	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
		printf("Invalid stream index.");
		return -1;
	}

	switch(pFormatCtx->streams[stream_index]->codecpar->codec_type) {
		case AVMEDIA_TYPE_VIDEO: {
				videoState->videoStream = stream_index;
				videoState->video_st = pFormatCtx->streams[stream_index];

				sInfo->analytics_ctx.height = pFormatCtx->streams[stream_index]->codecpar->height;
				sInfo->analytics_ctx.width = pFormatCtx->streams[stream_index]->codecpar->width;

				AVDictionaryEntry *tag = NULL;
				while ((tag = av_dict_get(pFormatCtx->streams[stream_index]->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
					if(strcmp(tag->key, "variant_bitrate") == 0) {
						sInfo->analytics_ctx.streamBandwidth = atoi(tag->value);
						break;
					}
				}
			}
			break;
		case AVMEDIA_TYPE_AUDIO: {
				videoState->audioStream = stream_index;
				videoState->audio_st = pFormatCtx->streams[stream_index];
			}
			break;
		default:
			break;
	}

	return 0;
}

void *playback_open(const char *filename) {
	AVFormatContext *fctx = NULL;

	StreamInfo *sinfo = new StreamInfo;
	sinfo->filename = filename;

	int ret;
	if((ret = openstream(&fctx,sinfo)) < 0) {
		delete sinfo;
		sinfo = nullptr;
		return nullptr;
	}
	
	AVLog::log_to_file("[Stream URL:%s]",sinfo->filename.c_str());

	int idxAud = -1;
	int idxVid = -1;

	for(int i=0;(i < fctx->nb_streams && (idxAud == -1 || idxVid == -1));i++) {
		if(fctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			idxVid = i;
		} else if(fctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			idxAud = i;
		}
	}

	if (stream_component_open(sinfo, idxVid) != 0) {
		AVLog::log_to_file("Unable to open video stream");
	}

	if (stream_component_open(sinfo, idxAud) != 0) {
		AVLog::log_to_file("Unable to open audio stream");
	}

	sinfo->analytics_ctx.videoDuration = fctx->duration;
	sinfo->bReadyToPlay = YES;
	sinfo->bStreamError = NO;
	if (sinfo->bPause == YES) {
		sinfo->bPause = NO;
		AVLog::log_to_file("UnPausing Stream-%d",sinfo->nStreamId);
	}
	AVLog::log_to_file("[Packet Read Started for Stream]");
	return sinfo;
}

int playback_start(void *ctx) {

	if(ctx == nullptr) {
		return -1;
	}
	StreamInfo *sinfo = (StreamInfo *)ctx;
	VideoState *videoState = &sinfo->videoState;	
	AVPacket * packet = av_packet_alloc();
	int64_t  prevAudioPTS = 0;
	int64_t  prevVideoPTS = 0;
	int64_t prevCurrentTimeTick = 0;
	int64_t startTime = av_gettime();
	int isBuferring = 1;
	int ret;
	while (1) {
		/*If Stream is Deleted then exit from the thread*/
		if (sinfo->bDeleteStream == YES) {
			break;
		}

		int64_t currentTimeTick =  av_gettime() - startTime;

		int64_t aBufferLevel = videoState->aBuffer - currentTimeTick;
		int64_t vBufferLevel = videoState->vBuffer - currentTimeTick;

		if(aBufferLevel < 1e+7 || vBufferLevel < 1e+7) {
			ret = av_read_frame(sinfo->videoState.formateContext,packet);

			if(ret < 0 && aBufferLevel <= 0 && vBufferLevel <= 0) {
				if(sinfo->bDeleteStream != YES ) {
					AVLog::log_to_file("Failed to read packet for Stream-[%d]",ret);
				}
				break;
			}
			if(ret == 0) {
				// read next frame also
				if(packet->stream_index == videoState->audioStream) {
					sinfo->analytics_ctx.decodedAFrame++;

					if(prevAudioPTS != 0) {
						double diff  = (packet->pts - prevAudioPTS)* av_q2d(videoState->audio_st->time_base) * 1e+6;
						videoState->aBuffer += diff;
					}

					prevAudioPTS = packet->pts;

				} else if(packet->stream_index == videoState->videoStream) {

					sinfo->analytics_ctx.decodedVFrame++;
					if(prevVideoPTS != 0) {
						double diff  = (packet->dts - prevVideoPTS)* av_q2d(videoState->video_st->time_base) * 1e+6;;
						videoState->vBuffer += diff;
					}
					prevVideoPTS = packet->dts;

				} else {
					//nUnknownPkts++;
				}

			}
		}

		if(isBuferring == 1) {
			// if Buffer Level is less then 4 sec then fill it
			if(!(aBufferLevel < 4e+6 && vBufferLevel < 4e+6)) {
				sinfo->analytics_ctx.bufferingCount++;
				isBuferring = 0;
				startTime = av_gettime();
			}
		}
		else if(ret != AVERROR_EOF && isBuferring == 0 && (videoState->aBuffer < currentTimeTick || videoState->vBuffer < currentTimeTick)) {
			sinfo->analytics_ctx.bufferingCount++;
			AVLog::log_to_file("Buffering [%f] [%f] [%f]\n", aBufferLevel/1e+6 ,vBufferLevel/1e+6 ,currentTimeTick/1e+6);
			isBuferring = 1;
		}


		if(isBuferring == 0 && (currentTimeTick - prevCurrentTimeTick) > 1e+6) {
			AVLog::log_to_file("Current time  [%f] [%f] [%f]\n",currentTimeTick/1e+6, aBufferLevel/1e+6 ,vBufferLevel/1e+6);
			sinfo->analytics_ctx.vBufferLevel = vBufferLevel/1e+6;
			analytics_log(sinfo);
			videoState->currentPosition = currentTimeTick/1e+6;
			prevCurrentTimeTick = currentTimeTick;
		}

		
		if(isBuferring == 0)
			usleep(5000);
		else
			usleep(1000);

		av_packet_unref(packet);
	}

	AVLog::log_to_file("[Deletion for StreamID]");
	if (sinfo->videoState.formateContext) {
		sinfo->bDeleteStream = YES;
		AVLog::log_to_file("[Closing StreamID]");
		avformat_close_input(&sinfo->videoState.formateContext);
	}
	return -1;
}


void playback_stop(void * ctx) {
	if(ctx == nullptr) {
		return;
	}
	StreamInfo *sinfo = (StreamInfo *)ctx;
	sinfo->bDeleteStream = YES;
	delete sinfo;
	sinfo = nullptr;
}