/*
 *  movepredicatefeature.cc
 *  src
 *
 *  Created by Hilmar Finnsson on 6/20/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include "movepredicatefeature.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::parsing;

/********* MOVEPREDICATEFEATURE ****************/

PredicateFeature* MovePredicateFeature::addQ(GameStateID i, double q)
{
	PredicateFeaturesItr itr = predicates.find(i);
	if(itr == predicates.end())
	{
		predicates[i].init(i, q);
		return &predicates[i];
	}
	else
	{
		itr->second.addQ(q);
		return &(itr->second);
	}
}
PredicateFeature* MovePredicateFeature::addQ(GameStateID i, double q, unsigned int n)
{
	PredicateFeaturesItr itr = predicates.find(i);
	if(itr == predicates.end())
	{
		predicates[i].init(i, q, n);
		return &predicates[i];
	}
	else
	{
		itr->second.addQ(q, n);
		return &(itr->second);
	}
}
double MovePredicateFeature::getQ(GameStateID i)
{
	PredicateFeaturesItr itr = predicates.find(i);
	if(itr == predicates.end())
	{
		predicates[i].setID(i);
		return MOVE_PREDICATE_EXPLORATION;
	}
	return itr->second.getQ();
}
unsigned int MovePredicateFeature::getVisits(GameStateID i)
{
	PredicateFeaturesItr itr = predicates.find(i);
	if(itr == predicates.end())
		return 0;
	return itr->second.getN();
}
double MovePredicateFeature::getConfidence(GameStateID i)
{
	PredicateFeaturesItr itr = predicates.find(i);
	if(itr == predicates.end())
	{
		predicates[i].setID(i);
		return MOVE_PREDICATE_EXPLORATION;
	}
	
	if(itr->second.getN() < PREDICATE_VISIT_THRESHOLD)
		return MOVE_PREDICATE_EXPLORATION;
	
	return predicates[i].getQ();
}

/********** MOVEPREDICATEFEATURES ***************/

double MovePredicateFeatures::getTotalQ(Move* m, GameState& s)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION * s.size();
	double total = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		total += getQ(m, s[n]);
	}
	return total;
}
double MovePredicateFeatures::getAverageQ(Move* m, GameState& s)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	double total = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		total += getQ(m, s[n]);
	}
	return total/s.size();
}
double MovePredicateFeatures::getMaxQ(Move* m, GameState& s)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	double max = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		if(max < getQ(m, s[n]))
		   max = getQ(m, s[n]);
	}
	return max;
}

double MovePredicateFeatures::getQ(Move* m, Compound* c)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	GameStateID moveid = theory->getGameStateID(m);
	MovePredicateFeatureItr itr = map.find(moveid);
	if(itr == map.end())
	{
		map[moveid].set(moveid, m);
		return MOVE_PREDICATE_EXPLORATION;
	}
	return itr->second.getQ(theory->getGameStateID(c));
}
double MovePredicateFeatures::getTotalConf(Move* m, GameState& s)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	double total = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		total += getConf(m, s[n]);
	}
	return total;
}
double MovePredicateFeatures::getAverageConf(Move* m, GameState& s)
{
	if(theory == NULL || !s.size())
		return MOVE_PREDICATE_EXPLORATION;
	double total = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		total += getConf(m, s[n]);
	}
	return total/s.size();
}
double MovePredicateFeatures::getMaxConf(Move* m, GameState& s)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	double max = 0.0;
	for(size_t n = 0 ; n < s.size() ; n++)
	{
		if(max < getConf(m, s[n]))
			max = getConf(m, s[n]);
	}
	return max;
}

double MovePredicateFeatures::getConf(Move* m, Compound* c)
{
	if(theory == NULL)
		return MOVE_PREDICATE_EXPLORATION;
	GameStateID moveid = theory->getGameStateID(m);
	MovePredicateFeatureItr itr = map.find(moveid);
	if(itr == map.end())
	{
		map[moveid].set(moveid, m);
		return MOVE_PREDICATE_EXPLORATION;
	}
	return itr->second.getConfidence(theory->getGameStateID(c));
}

void MovePredicateFeatures::addQ(Move* m, GameState* s, double q)
{
	if(theory == NULL)
		return;
	for(size_t n = 0 ; n < s->size() ; n++)
	{
		addQ(m, (*s)[n], q);
	}
}
void MovePredicateFeatures::addQ(Move* m, Compound* c, double q)
{
	if(theory == NULL)
		return;
	GameStateID moveid = theory->getGameStateID(m);
	MovePredicateFeatureItr itr = map.find(moveid);
	GameStateID cid = theory->getGameStateID(c);
	if(itr == map.end())
		map[moveid].set(moveid, m, cid, q);
	else
	{
		PredicateFeature* temp = itr->second.addQ(cid, q);
		if(temp->getN() > MAXMAP_THRESHOLD)
			updateMaxMap(cid, moveid, temp->getQ());
	}
	//CompoundStringBuffer::iterator citr = cbuf.find(cid);
	//if(citr == cbuf.end())
	//	cbuf[cid] = c->toString(theory->getSymbols());
}
void MovePredicateFeatures::addQ(GameStateID i, StateFacts& facts, double q)
{
	MovePredicateFeatureItr itr = map.find(i);
	if(itr == map.end())
	{
		map[i].setID(facts[0]);
		itr = map.find(i);
	}
	for(size_t n = 0 ; n < facts.size() ; n++)
	{
		PredicateFeature* temp = itr->second.addQ(facts[n], q);
		updateMaxMap(facts[n], i, temp->getQ());
	}
}
void MovePredicateFeatures::addQ(GameStateID i, StateFacts& facts, double q, unsigned int n)
{
	MovePredicateFeatureItr itr = map.find(i);
	if(itr == map.end())
	{
		map[i].setID(facts[0]);
		itr = map.find(i);
	}
	for(size_t f = 0 ; f < facts.size() ; f++)
	{
		PredicateFeature* temp = itr->second.addQ(facts[f], q, n);
		updateMaxMap(facts[f], i, temp->getQ());
	}
}

MaxPredicateAction* MovePredicateFeatures::getMaxPredicateAction(GameStateID p)
{
	MaxPredicateActionMapItr itr = maxMap.find(p);
	if(itr == maxMap.end())
		return NULL;
	else
		return &(itr->second);
	
}
void MovePredicateFeatures::updateMaxMap(GameStateID p, GameStateID a, double q)
{
	MaxPredicateActionMapItr itr = maxMap.find(p);
	if(itr == maxMap.end())
		maxMap[p].set(a,q);
	else if(itr->second.action == a || itr->second.qVal < q)
		itr->second.set(a,q);
}

/*********** INFO FUNCTIONS *****************/

void PredicateFeature::getInfo(std::ostream& infostream, CompoundStringBuffer& cbuf)
{
	CompoundStringBuffer::iterator citr = cbuf.find(id);
	if(citr == cbuf.end())
		infostream << id << " : \tn=" << n << " \tq=" << q;
	else
	{
		infostream << citr->second << " : \tn=" << n << " \tq=" << q;
		infostream << " \t[" << id << "]";
	}
}
void MovePredicateFeature::getInfo(std::ostream& infostream, CompoundStringBuffer& cbuf, GameTheory* theory)
{
	PredicateFeaturesItr itr;
	bool found = false;
	for(itr = predicates.begin() ; itr != predicates.end() ; itr++)
	{
		if(itr->second.getN() < PREDICATE_VISIT_THRESHOLD)
			continue;
		if(!found)
		{
			infostream << "Move:" << id;
			infostream << " [" << move->compound.toString(theory->getSymbols()) << "]";
			infostream << std::endl;
		}		
		found = true;
		itr->second.getInfo(infostream, cbuf);
		infostream << std::endl;
	}
}
void MovePredicateFeatures::getInfo(std::ostream& infostream)
{
	MovePredicateFeatureItr itr;
	infostream << "*** Current MovePredicateFeatures ***" << std::endl;
	for(itr = map.begin() ; itr != map.end() ; itr++)
	{
		itr->second.getInfo(infostream, cbuf, theory);
		infostream << std::endl;
	}
}
