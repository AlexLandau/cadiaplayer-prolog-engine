/*
 *  relationcounter.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/25/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "relationcounter.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::utils;

RelationCounter::RelationCounter(SymbolID name) :
m_name(name),
m_count(0)
{
}
RelationCounter::~RelationCounter()
{
	for(size_t parampos ; parampos < m_lists.size() ; parampos++)
	{
		m_lists[parampos].clear();
	}
	m_lists.clear();
	m_count=0;
	m_name=0;
}

SymbolID RelationCounter::getName()
{
	return m_name;
}
void RelationCounter::add(SymbolID relation, size_t parampos)
{
	if(m_lists.size() <= parampos)
		m_lists.resize(parampos+1);
	if(m_lists[parampos].size() <= relation)
		m_lists[parampos].resize(relation+1, 0);
	if(!m_lists[parampos][relation])
		m_count++;
	m_lists[parampos][relation]++;
}
size_t RelationCounter::getNumberOfRelations()
{
	return m_count;
}
size_t RelationCounter::getParamSize()
{
	return m_lists.size();
}
size_t RelationCounter::getCounterSize(size_t parampos)
{
	return m_lists[parampos].size();
}
size_t RelationCounter::getRelationCount(SymbolID relation, size_t parampos)
{
	if(parampos >= m_lists.size())
		return 0;
	return relation >= m_lists[parampos].size() ? 0 : m_lists[parampos][relation];
}
bool RelationCounter::matches(const RelationCounter& rc)
{
	if(m_count != rc.m_count)
		return false;
	
	if(m_lists.size() != rc.m_lists.size())
		return false;
	
	for(size_t i = 0 ; i < m_lists.size() ; i++)
	{
		if(m_lists[i].size() != rc.m_lists[i].size())
			return false;
		for(size_t j = 0 ; j < m_lists[i].size() ; j++)
		{
			if(m_lists[i][j] != rc.m_lists[i][j])
				return false;
		}
	}
	
	return true;
}

RelationCounterList& RelationCounter::getList(size_t parampos)
{
	return m_lists[parampos];
}
RelationCounters::RelationCounters()
{
}
RelationCounters::~RelationCounters()
{
	clear();
}

RelationCounter* RelationCounters::get(cadiaplayer::play::parsing::SymbolID name)
{
	RelationCounterMapItr itr = m_map.find(name);
	if(itr == m_map.end())
		return NULL;
	return itr->second;
}
RelationCounter* RelationCounters::add(cadiaplayer::play::parsing::SymbolID name, cadiaplayer::play::parsing::SymbolID relation, size_t parampos)
{
	RelationCounter* temp = get(name);
	if(!temp)
	{
		temp = new RelationCounter(name);
		m_map[name] = temp;
	}
	temp->add(relation, parampos);
	return temp;
}

void RelationCounters::getMatches(RelationCounterPairs& pairs)
{
	pairs.clear();
	RelationCounterMapItr firstItr;
	RelationCounterMapItr secondItr;
	RelationCounter* first = NULL;
	RelationCounter* temp = NULL;
	RelationCounter* second = NULL;
	int matches = 0;
	RelationCounterMap found;
	for(firstItr = m_map.begin() ; firstItr != m_map.end() ; firstItr++)
	{
		first = firstItr->second;
		matches = 0;
		for( secondItr = m_map.begin(); secondItr != m_map.end() ; secondItr++)
		{
			if(firstItr==secondItr)
				continue;
			temp = secondItr->second;
			
			if(first->matches(*temp))
			{
				second = temp;
				matches++;
			}
			if(matches > 1)
				break;
		}
		if(matches == 1)  // only accept pairs disjoint from everything else
		{
			if(found.find(first->getName()) == found.end() && found.find(second->getName()) == found.end())
			{
			   pairs.resize(pairs.size()+1);
			   pairs[pairs.size()-1].first = first;
			   pairs[pairs.size()-1].second = second;
			   found[first->getName()] = first;
			   found[second->getName()] = second;
			}
		}
	}
}

void RelationCounters::clear()
{
	RelationCounterMapItr itr;
	for(itr = m_map.begin() ; itr != m_map.end() ; itr++)
	{
		delete itr->second;
	}
	m_map.clear();
}

