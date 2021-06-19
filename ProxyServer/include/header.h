#pragma once

#include <string>
#include "headerUtils.h"
#include <vector>

std::vector<std::string> split(const std::string& str, const std::string& delim) noexcept
{
    std::vector<std::string> tokens = std::vector<std::string>();
    std::string strCopy = str;

    std::size_t pos = 0;
    std::string token;

    while ((pos = strCopy.find(delim)) != std::string::npos)
    {
        token = strCopy.substr(0, pos);
        strCopy.erase(0, pos + delim.length());

        tokens.push_back(token);
    }

    if (strCopy.length() > 0)
    {
        tokens.push_back(strCopy);
    }

    return tokens;
}

std::string concat(const std::vector<std::string>& strings, const std::string& delim = "") noexcept
{
    std::string result;

    for (std::size_t i = 0; i < strings.size(); i++)
    {
        result += strings[i];

        if ((i + 1) != strings.size())
        {
            result += delim;
        }
    }

    return result;
}

class Header {
 private:
        std::string key;
        std::string value;

    public:
        Header (const std::string& key, const std::string& value) noexcept: key(key), value(value)
        {
        }

        void set_value (const std::string& value) noexcept
        {
            this->value = value;
        }

        const std::string&  get_value() const noexcept
        {
            return this->value;
        }

        const std::string& get_key() const noexcept
        {
            return this->key;
        }

        std::string serialize() const noexcept
        {
            std::string header;
            header += this->key;
            header += ": ";
            header += this->value;
            header += HTTP::LINE_END;

            return header;
        }

        static Header deserialize(const std::string& header)
        {
            std::vector<std::string> segments = split(header, " ");

            const std::string key = segments[0].substr(0, segments[0].size() - 1);

            segments.erase(segments.begin());

            const std::string value = concat(segments, " ");

            return Header(key, value);
        }
};