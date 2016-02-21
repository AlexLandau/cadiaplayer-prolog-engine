/*
 *  logger.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/30/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include "logger.h"

using namespace cadiaplayer::utils;

Logger::Logger(std::string strLog, bool keepopen) : 
m_strLog(strLog), 
m_keepopen(keepopen)
{
	if(m_keepopen)
		m_log.open(strLog.c_str(), std::ios::app);
}

void Logger::log(std::string msg)
{
	if(!m_keepopen)
		m_log.open(m_strLog.c_str(), std::ios::app);
	
	m_log << Timer::getTimeStamp() << " : " << msg << std::endl;
	
	if(!m_keepopen)
		m_log.close();
}
void Logger::log(std::string msg, std::ostream& stream)
{
	stream << Timer::getTimeStamp() << " : " << msg << std::endl;
}
