
#include "Analytics.h"
#include <algorithm>

Analytics::Analytics() {
	decodedAFrame = 0;
	decodedVFrame = 0;
	bufferingCount = 0;
	corruptPacket = 0;
	width = 0;
	height= 0;
	streamBandwidth= 0;
	videoDuration= 0;
	vBufferLevel = 0;
}

void Analytics::populate_info() {
	add_to_json<double>("duration",videoDuration/1e+6);
	add_to_json<int>("width",width);
	add_to_json<int>("height",height);
	add_to_json<int>("bitrate",streamBandwidth);
	add_to_json<int>("buffering",bufferingCount);
	add_to_json<int>("corruptPacket",corruptPacket);
	add_to_json<int>("video_frame",decodedVFrame);
	add_to_json<int>("audio_frame",decodedAFrame);
	add_to_json<int>("bufferLevel",vBufferLevel);
}


void Analytics::add_error(std::string error) {
	if(error.find("Packet corrupt") != std::string::npos) {
		corruptPacket++;
    	//error_lst.push_back(error);
	}
}


std::string Analytics::to_string() {
	populate_info();
	
	std::string json = "{";
	for(auto pair : json_field) {
		json += "\""+pair.first+"\":"+pair.second+",";
	}
	json.pop_back();

	if(error_lst.size() != 0) {
		json += ",\"error_list\": [";
		for (std::string error : error_lst) {
			error.erase(std::remove(error.begin(), error.end(), '\n'), error.end());
			json += "\""+error+"\",";
		}
		json.pop_back();
		json += "]";
	}
	json += "}";
	return json;
}