/*
 *  shincupdmap.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef SHINCUPDMAP_H
#define SHINCUPDMAP_H

#include <iostream>
#include "../../utils/timer.h"
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#define MEGS100 104857600
#define UPDNODE_NOT_FOUND -1.0

namespace cadiaplayer
{
	namespace play 
	{
		namespace shmemutils
		{
			
			class UpdNode 
			{
			private:
				std::size_t m_visits;
				double m_val;
				boost::interprocess::interprocess_mutex mutex;
			public:
				UpdNode():m_visits(0),m_val(0.0){};
				void add(double val, std::string& error)
				{
					try
					{	
						boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
						++m_visits;
						m_val += (1.0/m_visits)*(val-m_val);
					}
					catch(...)
					{
						error = "Error caught when updating Incremental Update Node";
						std::cerr << error << std::endl;						
					}
				};
				double val(std::string& error)
				{
					double val = 0.0;
					try
					{
						boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(mutex);
						val = m_val;
					}
					catch(...)
					{
						error = "Error caught when reading Incremental Update Node";
						std::cerr << error << std::endl;
					}
					return val;
				};
			};
			
			class ShIncUpdMap
			{
				typedef int KeyType;
				typedef boost::interprocess::offset_ptr<UpdNode> MappedType;
				typedef std::pair<const KeyType, MappedType> ValueType;
				typedef boost::interprocess::allocator<ValueType, boost::interprocess::managed_shared_memory::segment_manager>  ShmemAllocator;
				typedef boost::interprocess::map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> UpdMap;
				
			private:
				boost::interprocess::managed_shared_memory* m_segment;
				bool m_destroyOnExit;
				std::string m_name;
				UpdMap* m_map;
				std::string m_logbuf;
				boost::interprocess::named_mutex* m_mutex;
			public:
				ShIncUpdMap();
				~ShIncUpdMap();
				
				void log(std::string str);
				void create(boost::interprocess::managed_shared_memory& segment, std::string name);
				void open(boost::interprocess::managed_shared_memory& segment, std::string name);
				void destroy(boost::interprocess::managed_shared_memory& segment);
				void reset();
				void add(KeyType key, double data);
				double get(KeyType key);
				double get(KeyType key, double notfoundval);
			};
		}
	}
}

			
#endif // SHINCUPDMAP_H

