#ifndef __ANALYTICS_H__
#define __ANALYTICS_H__

#include <map>
#include <vector>
#include <string>
class Analytics {

private:
	void populate_info();
public:
	int decodedAFrame;
	int decodedVFrame;
	int bufferingCount;
	int height;
	int streamBandwidth;
	double videoDuration;
	int corruptPacket;
	int vBufferLevel;
	int width;
	Analytics();
	std::map<std::string, std::string> json_field;
	std::vector<std::string> error_lst;

	template<typename T>
	void add_to_json(std::string key, T value) {
		json_field[key] = std::to_string(value);
	}

	void add_to_json(std::string key, std::string value) {
		json_field[key] = "\""+value+"\"";
	}

 	void add_error(std::string error);
	std::string to_string();
};

#endif // __ANALYTICS_H__