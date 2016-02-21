/*
 *  shkeymap.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "shkeymap.h"


using namespace boost::interprocess;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::shmemutils;


ShStateIDMap::ShStateIDMap()
{
}
ShStateIDMap::~ShStateIDMap()
{
	if(m_destroyOnExit)
	{
		m_segment->destroy<ShGameStateIDMap>(m_name.c_str());
	}
}

uint32_t ShStateIDMap::getMemoryNeed(std::size_t listsize, std::size_t mapsize, std::string name)
{
	uint32_t mem = sizeof(ShStateID) * listsize * mapsize;
	mem += sizeof(boost::interprocess::managed_shared_memory*);
	mem += sizeof(m_destroyOnExit);
	mem += (sizeof(char) * 2 * name.size());
	mem += sizeof(std::size_t*);
	mem += sizeof(ShGameStateIDMap*);
	return 2*mem;
}

void ShStateIDMap::create(std::size_t listsize, std::size_t mapsize, boost::interprocess::managed_shared_memory& segment, std::string name)
{
	m_segment = &segment;
	ShGameStateIDAllocator alloc_inst (segment.get_segment_manager());
	
	m_map = segment.construct<ShGameStateIDMap>(name.c_str())(alloc_inst);
	std::string listStr = name + LISTSIZE_EXT;
	m_mapsize = segment.construct<std::size_t>(listStr.c_str())(mapsize);
	
	for (size_t i = 0 ; i < listsize ; i++) 
	{
		for (size_t j = 0 ; j < mapsize ; j++) 
		{
			m_map->push_back((((ShStateID)rand()) << 32) | rand());
		}
	}
	
	m_destroyOnExit = true;
	m_name = name;
}
void ShStateIDMap::open(boost::interprocess::managed_shared_memory& segment, std::string name)
{
	m_segment = &segment;
	m_map = segment.find<ShGameStateIDMap>(name.c_str()).first;
	std::string listStr = name + LISTSIZE_EXT;
	m_mapsize = segment.find<std::size_t>(listStr.c_str()).first;
	m_destroyOnExit = false;
	m_name = name;
}

void ShStateIDMap::destroy(boost::interprocess::managed_shared_memory& segment)
{
	segment.destroy<ShGameStateIDMap>(m_name.c_str());
}

uint64_t ShStateIDMap::get(std::size_t listsize, std::size_t mapsize)
{
	std::size_t pos = (*m_mapsize) * listsize + mapsize; 
	return (*m_map)[pos];
}
