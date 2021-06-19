#pragma once

#include <map>
#include "header.h"
#include <pthread.h>

using namespace HTTP;
class Request
{
    private:
        Version version;
        Method method;
        std::string resource;
        std::map<std::string, Header> headers;

    public:
        std::string getHeaderValue(std::string key) {
            std::string str;
            for (auto c : headers) {
                if(c.first == key) {
                    str =  c.second.get_value();
                    break;
                }
            }
            return str;    
        }

        Request(Method method, const std::string& resource, const std::map<std::string, Header>& headers, Version version = Version::HTTP_1_1) noexcept: version(version), method(method), resource(resource), headers(headers)
        {
        }

        std::string serialize() const noexcept
        {
            std::string request;
            request += to_string(this->method);
            request += " ";
            request += this->resource;
            request += " ";
            request += to_string(this->version);
            request += LINE_END;

            for (const std::pair<const std::string, Header>& header : this->headers)
            {
                request += header.second.serialize();
            }

            request += LINE_END;
            return request;
        }

        
        static Request deserialize(const std::string request)
        {
            std::vector<std::string> lines = split(request, std::string(LINE_END));

            if (lines.size() < 1)
            {
                throw std::runtime_error("HTTP Request ('" + std::string(request) + "') consisted of " + std::to_string(lines.size()) + " lines, should be >= 1.");
            }

            std::vector<std::string> segments = split(lines[0], " ");

            if (segments.size() != 3)
            {
                throw std::runtime_error("First line of HTTP request ('" + std::string(request) + "') consisted of " + std::to_string(segments.size()) + " space separated segments, should be 3.");
            }

            const Method method = method_from_string(segments[0]);
            const std::string resource = segments[1];
            const Version version = version_from_string(segments[2]);

            std::map<std::string, Header> headers;

            for (std::size_t i = 1; i < lines.size(); i++)
            {
                if (lines[i].size() > 0)
                {
                    const Header header = Header::deserialize(lines[i]);
                    headers.insert(std::make_pair(header.get_key(), header));
                }
            }

            return Request(method, resource, headers, version);
        }
};