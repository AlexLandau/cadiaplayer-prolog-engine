/*
 *  movepredicatefeature.h
 *  src
 *
 *  Created by Hilmar Finnsson on 6/20/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef MOVE_PREDICATE_FEATURE_H
#define MOVE_PREDICATE_FEATURE_H

#define MOVE_PREDICATE_NOT_FOUND -1.0
#define MOVE_PREDICATE_EXPLORATION GOAL_DRAW

#include "../gametheory.h"
#include "../move.h"
#include <ext/hash_map>

namespace cadiaplayer {
	namespace play {
		namespace utils {
			const unsigned int PREDICATE_VISIT_THRESHOLD = 10;
			const unsigned int MAXMAP_THRESHOLD = 10;
			
			typedef cadiaplayer::play::GameStateID		GameStateID;
			typedef cadiaplayer::utils::LongLongHash	LongLongHash;
			typedef __gnu_cxx::hash_map<GameStateID, size_t, LongLongHash> ActionIndexMap;
			typedef ActionIndexMap::iterator ActionIndexMapItr;
			typedef __gnu_cxx::hash_map<GameStateID, std::string, LongLongHash> CompoundStringBuffer;
			
			struct MaxPredicateAction
			{
				GameStateID action;
				double qVal;
				void set(GameStateID a, double q){action=a;qVal=q;}; 
			};
			typedef __gnu_cxx::hash_map<GameStateID, MaxPredicateAction, LongLongHash> MaxPredicateActionMap;
			typedef MaxPredicateActionMap::iterator MaxPredicateActionMapItr;

			class PredicateFeature
				{
				private:
					GameStateID id;
					unsigned int n;
					double q;
				public:
					PredicateFeature(){id=0;n=0;q=0.0;};
					PredicateFeature(GameStateID i){id=i;n=0;q=0.0;};
					void init(GameStateID i, double initQ){id=i;n=1;q=initQ;};
					void init(GameStateID i, double initQ, unsigned int count){id=i;n=count;q=initQ;};
					void setID(GameStateID i){id=i;};
					GameStateID getID(){return id;}
					unsigned int getN(){return n;};
					double getQ(){if(n)return q;return MOVE_PREDICATE_EXPLORATION;};
					double addQ(double val){q+=1.0/++n*(val-q); return q;};
					double addQ(double val, unsigned int count){if(!count)return q;n+=count;q+=count/n*(val-q); return q;};
					
					void getInfo(std::ostream& infostream, CompoundStringBuffer& cbuf);
				};
			typedef __gnu_cxx::hash_map<StateFactID, PredicateFeature, LongLongHash> PredicateFeatures;
			typedef PredicateFeatures::iterator PredicateFeaturesItr;
			
			class MovePredicateFeature
				{
				private:
					GameStateID id;
					Move* move;
					PredicateFeatures predicates;
				public:
					MovePredicateFeature(){id=0;move=NULL;};
					MovePredicateFeature(GameStateID i){id=i;move=NULL;};
					
					GameStateID getID(){return id;};
					PredicateFeatures& getPredicates(){return predicates;};
					
					void setID(GameStateID i){id=i;};
					void setMove(Move* m){move=m;};
					void set(GameStateID i, Move* m){id=i;move=m;};
					void set(GameStateID i, Move* m, GameStateID ci, double q){set(i,m);addQ(ci,q);};
					Move* getMove(){return move;};
					
					PredicateFeature* addQ(GameStateID i, double q);
					PredicateFeature* addQ(GameStateID i, double q, unsigned int n);
					double getQ(GameStateID i);
					unsigned int getVisits(GameStateID i);
					double getConfidence(GameStateID i);
					
					void getInfo(std::ostream& infostream, CompoundStringBuffer& cbuf, GameTheory* theory);
				};
			
			typedef __gnu_cxx::hash_map<GameStateID, MovePredicateFeature, LongLongHash> MovePredicateFeatureMap;
			typedef MovePredicateFeatureMap::iterator MovePredicateFeatureItr;
			
			class MovePredicateFeatures
				{
				private:
					GameTheory* theory;
					MovePredicateFeatureMap map;
					MaxPredicateActionMap maxMap;
					CompoundStringBuffer cbuf;
				public:
					MovePredicateFeatures(){theory=NULL;};
					MovePredicateFeatures(GameTheory* t){theory=t;};
					
					void setTheory(GameTheory* t){theory=t;};
					void clear(){map.clear();maxMap.clear();};
					
					double getTotalQ(Move* m, GameState& s);
					double getAverageQ(Move* m, GameState& s);
					double getMaxQ(Move* m, GameState& s);
					double getQ(Move* m, cadiaplayer::play::parsing::Compound* c);
					
					double getTotalConf(Move* m, GameState& s);
					double getAverageConf(Move* m, GameState& s);
					double getMaxConf(Move* m, GameState& s);
					double getConf(Move* m, cadiaplayer::play::parsing::Compound* c);
					
					void addQ(Move* m, GameState* s, double q);
					void addQ(Move* m, cadiaplayer::play::parsing::Compound* c, double q);
					void addQ(GameStateID i, StateFacts& facts, double q);
					void addQ(GameStateID i, StateFacts& facts, double q, unsigned int n);
					
					MaxPredicateAction* getMaxPredicateAction(GameStateID p);
					void updateMaxMap(GameStateID p, GameStateID a, double q);

					void getInfo(std::ostream& infostream);
				};
			
			typedef std::vector<MovePredicateFeatures> MovePredicateFeaturesList;
			typedef MovePredicateFeaturesList::iterator MovePredicateFeaturesListItr;
			
			static void clearMovePredicateFeaturesList(MovePredicateFeaturesList& list)
			{
				for(MovePredicateFeaturesListItr itr = list.begin() ; itr != list.end() ; itr++)
				{
					itr->clear();
				}
			}
		}
	}
}
#endif
