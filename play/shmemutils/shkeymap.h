/*
 *  shkeymap.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */


#ifndef SHSTATEIDMAP_H
#define SHSTATEIDMAP_H

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#define LISTSIZE_EXT "_listsize"

namespace cadiaplayer
{
	namespace play 
	{
		namespace shmemutils
		{
			
			class ShStateIDMap
			{
				typedef uint64_t ShStateID;
				typedef boost::interprocess::allocator<ShStateID, boost::interprocess::managed_shared_memory::segment_manager> ShGameStateIDAllocator;
				typedef boost::interprocess::vector<ShStateID, ShGameStateIDAllocator> ShGameStateIDMap;
				
			private:
				boost::interprocess::managed_shared_memory* m_segment;
				bool m_destroyOnExit;
				std::string m_name;
				std::size_t* m_mapsize;
				ShGameStateIDMap* m_map;
			public:
				ShStateIDMap();
				~ShStateIDMap();
				
				uint32_t getMemoryNeed(std::size_t listsize, std::size_t mapsize, std::string name);
				void create(std::size_t listsize, std::size_t mapsize, boost::interprocess::managed_shared_memory& segment, std::string name);
				void open(boost::interprocess::managed_shared_memory& segment, std::string name);
				void destroy(boost::interprocess::managed_shared_memory& segment);
				ShStateID get(std::size_t listpos, std::size_t mappos);
			};
		}
	}
}


#endif // SHSTATEIDMAP_H

