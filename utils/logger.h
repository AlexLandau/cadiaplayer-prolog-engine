/*
 *  logger.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/30/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include "timer.h"

namespace cadiaplayer {
	namespace utils {
		
		class Logger
			{
			private:
				std::string m_strLog;
				std::ofstream	m_log;
				bool		m_keepopen;
			public:
				Logger(std::string strLog, bool keepopen = false);
				void log(std::string msg);
				void log(std::string msg, std::ostream& stream);  
				~Logger(){if(m_keepopen) m_log.close();};
			};
	}
}
#endif // LOGGER_H
