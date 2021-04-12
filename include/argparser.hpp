#pragma once

#include <algorithm>
#include <string>


class argument_parser
{
public:
    argument_parser(int argc, char **argv) : begin(argv) , end(argv + argc) { };

    std::string get_string(const std::string &option, const std::string def = "") const
    {
        char **itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) return std::string(*itr);
        return def;
    }

    int get_int(const std::string &option, const int def = 0) const
    {
        char **itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) return (int)std::stof(*itr);
        return def;
    }

    int get_long(const std::string &option, const long def = 0) const
    {
        char **itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) return (long)std::stold(*itr);
        return def;
    }

    bool exists(const std::string &option) const
    {
      return std::find(begin, end, option) != end;
    }

private:
    char **begin, **end;
};
