#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <string.h>
#include <sstream>
#include <stdio.h>

namespace cadiaplayer{
	namespace utils{
		
#define USE_TIME_STRUCT
		
		class Timer
			{
			private:
#ifdef USE_TIME_STRUCT
				std::time_t starttime;
				std::clock_t startclock;
				std::time_t endtime;
#else
				std::clock_t starttime;
				std::string measure;
				int div;
#endif
			public:
				Timer() 
				{ 
#ifndef USE_TIME_STRUCT
					if(CLOCKS_PER_SEC < 1000)
					{
						measure = "tics";
						div = 1;
					}
					else
					{
						measure = "msecs";
						div = CLOCKS_PER_SEC/1000;
					}
#endif
				}
				void startTimer() 
#ifdef USE_TIME_STRUCT
				{time (&starttime);startclock=std::clock();};
#else
				{starttime = std::clock();};
#endif
				bool hasTime(double seconds)
#ifdef USE_TIME_STRUCT
				{ std::time(&endtime); return std::difftime(endtime,starttime) < seconds;};
#else
				{ return (std::clock()-starttime)/CLOCKS_PER_SEC < seconds;};
#endif
				double remaining(double seconds)
#ifdef USE_TIME_STRUCT
				{ time(&endtime); return seconds - difftime (endtime,starttime);};
#else
				{ return seconds - (clock()-starttime)/CLOCKS_PER_SEC;};
#endif
				int getTicsElapsed()
#ifdef USE_TIME_STRUCT
				{return clock()-startclock;};
#else
				{return clock()-starttime;};
#endif
				int secToTics(int sec){return sec*CLOCKS_PER_SEC;};
				
				std::string getElapsedTime(void)
				{
					char buffer[128];
#ifdef USE_TIME_STRUCT
					std::time(&endtime);
					int sec = static_cast<int>(difftime (endtime,starttime));
					sprintf(buffer, "Elapsed time %d seconds", sec);
#else
					std::clock_t endtime = std::clock();
					int tics = (endtime-starttime);
					std::sprintf(buffer, "Elapsed time %d %s", tics/div, measure.c_str());
#endif
					std::string time = buffer;
					return time;
				};
				
				std::string getElapsedClock(void)
				{
					char buffer[128];
#ifdef USE_TIME_STRUCT
					std::time(&endtime);
					int sec = static_cast<int>(difftime (endtime,starttime));
#else
					std::clock_t endtime = std::clock();
					int tics = (endtime-starttime);
					int sec = tics/CLOCKS_PER_SEC;
#endif
					int hour = sec/3600;
					sec -= (hour*3600);
					int min = sec/60;
					sec -= (min*60);
					sprintf(buffer, "Elapsed time %d:%d:%d", hour, min, sec);
					std::string time = buffer;
					return time;
				};
				
				static std::string getTimeStamp()
				{
					std::time_t tm;
					std::time(&tm);
					std::string timestamp = ctime(&tm);
					timestamp[strlen(timestamp.c_str())-1] = ' ';
					return timestamp;
				}
				static std::string getPGNDate()
				{
					std::time_t tm;
					std::time(&tm);
					struct tm * timeinfo;
					timeinfo = std::localtime(&tm);
					std::stringstream ss;
					ss << (timeinfo->tm_year + 1900) << ".";
					if(timeinfo->tm_mon+1 < 10)
						ss << "0";
					ss << (timeinfo->tm_mon+1) << ".";
					if(timeinfo->tm_mday < 10)
						ss << "0";
					ss << (timeinfo->tm_mday);
					return ss.str();
				}
			};
	}} // namespaces
#endif // TIMER_H
