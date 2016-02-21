/*
 *  untitled.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "shincupdmap.h"

using namespace boost::interprocess;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::shmemutils;

ShIncUpdMap::ShIncUpdMap():
m_mutex(NULL)
{
}
ShIncUpdMap::~ShIncUpdMap()
{
	if(m_destroyOnExit)
	{
		reset();
		destroy(*m_segment);
	}
}
void ShIncUpdMap::log(std::string str)
{
	std::ofstream o;
	o.open("ShIncUpdMap.log", std::ios::app);
	o << cadiaplayer::utils::Timer::getTimeStamp() << " ";
	o << str.c_str() << "\n";
	o.close();
}
void ShIncUpdMap::create(boost::interprocess::managed_shared_memory& segment, std::string name)
{
	m_segment = &segment;
	ShmemAllocator alloc_inst (segment.get_segment_manager());
	
	segment.destroy<UpdMap>(m_name.c_str());
	m_map = segment.construct<UpdMap>(name.c_str())(std::less<int>(), alloc_inst);
	boost::interprocess::named_mutex::remove("addlock");
	m_mutex = new named_mutex(create_only, "addlock");
	
	m_destroyOnExit = true;
	m_name = name;
}
void ShIncUpdMap::open(boost::interprocess::managed_shared_memory& segment, std::string name)
{
	m_segment = &segment;
	m_map = segment.find<UpdMap>(name.c_str()).first;
	m_mutex = new named_mutex(open_only, "addlock");
	
	m_destroyOnExit = false;
	m_name = name;
}
void ShIncUpdMap::destroy(boost::interprocess::managed_shared_memory& segment)
{
	segment.destroy<UpdMap>(m_name.c_str());
	boost::interprocess::named_mutex::remove("addlock");
	if(m_mutex)
		delete m_mutex;
}
void ShIncUpdMap::reset()
{
	for(UpdMap::iterator itr = m_map->begin() ; itr != m_map->end() ; itr++)
	{
		m_segment->deallocate(itr->second.get());
	}
	m_map->clear();
}
void ShIncUpdMap::add(KeyType key, double data)
{	
	try 
	{
		scoped_lock<named_mutex> lock(*m_mutex);
		
		UpdMap::iterator itr = m_map->find(key);
		if(itr != m_map->end())
		{
			m_logbuf.clear();
			itr->second->add(data, m_logbuf);
		}
		else
		{
			MappedType temp = static_cast<UpdNode*>(m_segment->allocate(sizeof(UpdNode)));
			m_logbuf.clear();
			temp->add(data, m_logbuf);
			(*m_map)[key] = temp;
		}
	}
	catch (...) 
	{
		std::string error = "Error caught when adding to a Shared Incremental Update Map";
		std::cerr << error << std::endl;
		log(error);
	}
}
double ShIncUpdMap::get(KeyType key) 
{
	return get(key, UPDNODE_NOT_FOUND);
	
}
double ShIncUpdMap::get(KeyType key, double notfoundval)
{
	double val = notfoundval;
	try 
	{
		UpdMap::iterator itr = m_map->find(key);
		if(itr != m_map->end())
		{
			m_logbuf.clear();
			val = itr->second->val(m_logbuf);
			if(m_logbuf.size())
				log(m_logbuf);
		}
	}
	catch (...) 
	{
		std::string error = "Error caught when getting from a Shared Incremental Update Map";
		std::cerr << error << std::endl;
		log(error);
	}	
	return val;
}
