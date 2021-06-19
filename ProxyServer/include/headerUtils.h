#pragma once
#include <string>
namespace HTTP {
    constexpr static std::string_view LINE_END = "\r\n";
    enum class Method {
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };

    enum class Version
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0
    };

    std::string to_string(Version version) {
        switch(version)
        {
            case Version::HTTP_1_0:
                return "HTTP/1.0";

            case Version::HTTP_1_1:
                return "HTTP/1.1";

            case Version::HTTP_2_0:
                return "HTTP/2.0";
        }
    }

    Version version_from_string (const std::string& version) noexcept
    {
        if (version == to_string(Version::HTTP_1_0))
        {
            return Version::HTTP_1_0;
        }
        else if (version == to_string(Version::HTTP_1_1))
        {
            return Version::HTTP_1_1;
        }
        else if (version == to_string(Version::HTTP_2_0))
        {
            return Version::HTTP_2_0;
        }
    }

    std::string to_string(Method method) {
        switch(method)
        {
            case Method::GET:
                return "GET";
            case Method::HEAD:
                return "HEAD";
            case Method::POST:
                return "POST";
            case Method::PUT:
                return "PUT";
            case Method::DELETE:
                return "DELETE";
            case Method::TRACE:
                return "TRACE";
            case Method::OPTIONS:
                return "OPTIONS";
            case Method::CONNECT:
                return "CONNECT";
            case Method::PATCH:
                return "PATCH";
        }
    }
    
    Method method_from_string (const std::string& method) noexcept {
        if (method == to_string(Method::GET))
        {
            return Method::GET;
        }
        else if (method == to_string(Method::HEAD))
        {
            return Method::HEAD;
        }
        else if (method == to_string(Method::POST))
        {
            return Method::POST;
        }
        else if (method == to_string(Method::PUT))
        {
            return Method::PUT;
        }
        else if (method == to_string(Method::DELETE))
        {
            return Method::DELETE;
        }
        else if (method == to_string(Method::TRACE))
        {
            return Method::TRACE;
        }
        else if (method == to_string(Method::OPTIONS))
        {
            return Method::OPTIONS;
        }
        else if (method == to_string(Method::CONNECT))
        {
            return Method::CONNECT;
        }
        else if (method == to_string(Method::PATCH))
        {
            return Method::PATCH;
        }
    }
};