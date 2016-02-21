/*
 *  relationcounter.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/25/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef RELATION_COUNTER_H
#define RELATION_COUNTER_H

#include <ext/hash_map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "../../utils/longlonghash.h"
#include "../parsing/symtab.h"

namespace cadiaplayer{
	namespace play{
		namespace utils{
			
			typedef std::vector<cadiaplayer::play::parsing::SymbolID> RelationCounterList;
			typedef std::vector<RelationCounterList> RelationCounterLists;
			
			class RelationCounter
				{
				private:
					cadiaplayer::play::parsing::SymbolID m_name;
					size_t m_count;
					RelationCounterLists m_lists;
				public:
					RelationCounter(cadiaplayer::play::parsing::SymbolID name);
					~RelationCounter();
					
					cadiaplayer::play::parsing::SymbolID getName();
					void add(cadiaplayer::play::parsing::SymbolID relation, size_t parampos);
					size_t getNumberOfRelations();
					size_t getParamSize();
					size_t getCounterSize(size_t parampos);
					size_t getRelationCount(cadiaplayer::play::parsing::SymbolID relation, size_t parampos);
					RelationCounterList& getList(size_t parampos);
					bool matches(const RelationCounter& rc);
				};
			
			struct RelationCounterPair
			{
				RelationCounter* first;
				RelationCounter* second;
			};
			typedef std::vector<RelationCounterPair> RelationCounterPairs;
			
			typedef __gnu_cxx::hash_map<cadiaplayer::play::parsing::SymbolID, RelationCounter*, cadiaplayer::utils::LongLongHash>	RelationCounterMap;
			typedef RelationCounterMap::iterator RelationCounterMapItr;
			class RelationCounters
				{
				private:
					RelationCounterMap m_map;
				public:
					RelationCounters();
					~RelationCounters();
					
					RelationCounter* get(cadiaplayer::play::parsing::SymbolID name);
					RelationCounter* add(cadiaplayer::play::parsing::SymbolID name, cadiaplayer::play::parsing::SymbolID relation, size_t parampos);
					
					void getMatches(RelationCounterPairs& pairs);
					
					void clear();
					
				};
			
		}
	}
}
#endif // RELATION_COUNTER_H
