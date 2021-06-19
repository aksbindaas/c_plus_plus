#pragma once
#include <map>
#include "header.h"
using namespace HTTP;

class Response
{
    private:
        int responseCode;
        Version version;
        std::map<std::string, Header> headers;
        std::string body;

    public:
        constexpr static int OK = 200;
        constexpr static int CREATED = 201;
        constexpr static int ACCEPTED = 202;
        constexpr static int NO_CONTENT = 203;
        constexpr static int BAD_REQUEST = 400;
        constexpr static int FORBIDDEN = 403;
        constexpr static int NOT_FOUND = 404;
        constexpr static int REQUEST_TIMEOUT = 408;
        constexpr static int INTERNAL_SERVER_ERROR = 500;
        constexpr static int BAD_GATEWAY = 502;
        constexpr static int SERVICE_UNAVAILABLE = 503;

        Response (int responseCode, Version version, const std::map<std::string, Header>& headers, const std::string& body) noexcept: responseCode(responseCode), headers(headers), body(body)
        {
        }

        int get_response_code() const noexcept
        {
            return this->responseCode;
        }

        const std::string& get_body() const noexcept
        {
            return this->body;
        }

        const std::map<std::string, Header> get_headers() const noexcept
        {
            return this->headers;
        }

        static Response deserialize(const std::string& response) noexcept
        {
            std::vector<std::string> segments = split(response, std::string(LINE_END) + std::string(LINE_END));

            std::string headerSegment = segments[0];
            segments.erase(segments.begin());

            std::string body = concat(segments);

            std::vector<std::string> headerLines = split(headerSegment, std::string(LINE_END));

            const std::string& responseCodeLine = headerLines[0];

            std::vector<std::string> responseCodeSegments = split(responseCodeLine, " ");

            Version version = version_from_string(responseCodeSegments[0]);
            int responseCode = std::stoi(responseCodeSegments[1]);

            headerLines.erase(headerLines.begin());

            std::map<std::string, Header> headers;

            for (const std::string& line : headerLines)
            {
                const Header header = Header::deserialize(line);
                headers.insert(std::make_pair(header.get_key(), header));
            }

            return Response(responseCode, version, headers, body);
        }
};