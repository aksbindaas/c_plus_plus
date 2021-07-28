#include <stdio.h>
#include "PlayerManager.h"

#include <iostream>

void analytics_data(std::string str) {
	std::cout<<str<<std::endl;
}

int main(int argc, char * argv[]) {
	char *filename = nullptr;
	char *stream_url = nullptr;
    if (argc != 3) {
		printf("Usage: ./clientApp <stream-url> <log_file_name>\n");
		// Configure Default
		stream_url = (char *)"http://127.0.0.1/4KBigBuckBunny/master.m3u8";
		filename = (char *)"test.txt";
	//	goto exitmain;
	} else {
		stream_url = argv[1];
		filename = argv[2];
	}

	PlaybackContext *ctx = load(stream_url, filename);
	register_analyticsCB(&analytics_data);
	start(ctx);
	
	stop(ctx);

    return 0;
}