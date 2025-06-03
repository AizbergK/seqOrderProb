#ifndef TIMER_HPP
#define TIMER_HPP
import std;

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

class TimerUtility
{

public:
    void startTimer()
    {
        start = high_resolution_clock::now();
    }
    std::u8string getDuration(std::u8string comment = u8"")
    {
        setEndTime();
        setDuration();
        return sendMessage(comment);
    }

private:
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    duration<double, std::nano> duration_ns;
    void setEndTime()
    {
        end = high_resolution_clock::now();
    }
    void setDuration()
    {
        duration_ns = end - start;
    }
    std::u8string sendMessage(std::u8string& comment)
    {
        std::locale::global(std::locale("en_US.UTF-8"));
        std::u8string message = u8"";

        if (duration_ns.count() >= 1'000'000'000)
        {
            double duration_s = duration_ns.count() / 1'000'000'000;
            std::string temp_string = std::to_string(duration_s);
            message += std::u8string(temp_string.begin(), temp_string.end());
            message = message.substr(0, 6);
            message += u8"s";
        }
        else if (duration_ns.count() >= 1'000'000)
        {
            double duration_ms = duration_ns.count() / 1'000'000;
            std::string temp_string = std::to_string(duration_ms);
            message += std::u8string(temp_string.begin(), temp_string.end());
            message = message.substr(0, 6);
            message += u8"ms";
        }
        else if (duration_ns.count() >= 1'000)
        {
            double duration_us = duration_ns.count() / 1'000;
            std::string temp_string = std::to_string(duration_us);
            message += std::u8string(temp_string.begin(), temp_string.end());
            message = message.substr(0, 6);
            message += u8"µs ";
        }
        else
        {
            std::string temp_string = std::to_string(duration_ns.count());
            message += std::u8string(temp_string.begin(), temp_string.end());
            message = message.substr(0, 6);
            message += u8"ns";
        }
        if (comment.length())
        {
            message.append(10 - message.length(), ' ');
            message += u8": " + comment;
        }
        message += '\n';
        return message;
        message.clear();
    }
};

#endif