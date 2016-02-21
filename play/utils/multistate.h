/*
 *  multistate.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 12/9/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef MULTISTATE_H
#define MULTISTATE_H

#include <stdlib.h>
#include <vector>
#include <stack>
#include <queue>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>
#include <ext/hash_map>
#include "../gametheory.h"

#define MM_INIT_OPTI 100
#define MM_INIT_PESS 0
#define EQUIV_PARAM 500

//#define USE_MOVING_AVERAGES
#ifdef USE_MOVING_AVERAGES
#define MOVING_AVERAGES_THRESHOLD 50
#define MOVING_AVERAGES_STEPSIZE 0.05
#endif 

#define USE_MCTS_SOLVER

namespace cadiaplayer{
	namespace play {
		namespace utils {
			
			typedef GameStateID MTStateID;
			typedef GameStateID MTActionID;
			typedef	std::size_t Visit;
			typedef	Visit Visits;
			typedef double	QValue;
			typedef std::vector<QValue>	QValues;
			typedef std::size_t Depth;
			struct MTState;
			struct MTNode;
			enum StationaryPointType{
				Minima, 
				Maxima,
				Inflection
			};
			
			struct MTAction
			{
				MTActionID id;
				Visits n;
				QValue q; 
				QValue max;
				MTState* state;
				MTNode* node; // If this action is a turntaking one there is always only one matching node
				Move* move;
				bool pruned;
				//RAVE
				Visits nRAVE;
				QValue qRAVE; 
				
				MTAction(MTActionID aid, MTState* s, Move* m):
				id(aid), n(0), q(0), max(0), state(s), node(NULL), move(m), pruned(false),
				nRAVE(0), qRAVE(0)
				{};
				void reset(MTActionID aid, MTState* s, Move* m)
				{
					id=aid;n=0;q=0;max=0;state=s;node=NULL;move=m;pruned=false;nRAVE=0;qRAVE=0;
				};
			};
			typedef __gnu_cxx::hash_map<MTActionID, Move*, cadiaplayer::utils::LongLongHash>	MTActionBuffer;
			typedef MTActionBuffer::iterator													MTActionBufferItr;
			typedef	std::vector<MTAction*>														MTRoleActions;
			typedef MTRoleActions::iterator														MTRoleActionsItr;
			typedef	std::vector<QValue>															Goals;
			
			class MTNode
			{
			public:
				MTState* parent;
				MTRoleActions actions;
				MTState* child;
				Goals goals;
				bool expanded;
			private:
				int toTerminal;
			public:
				MTNode():parent(NULL), child(NULL), toTerminal(TERMINAL_UNKNOWN), expanded(false){};
				void reset(){parent=NULL;child=NULL;toTerminal=TERMINAL_UNKNOWN;actions.clear();goals.clear();expanded=false;};
				bool isUnknown(){return toTerminal==TERMINAL_UNKNOWN;};
				bool isTerminal(){return toTerminal==TERMINAL_TRUE;};
				bool isNonTerminal(){return toTerminal==TERMINAL_FALSE;};
				int  getTerminal(){return toTerminal;};
				void makeTerminal(){toTerminal=TERMINAL_TRUE;};
				void makeNonTerminal(){toTerminal=TERMINAL_FALSE;};
				QValue getGoal(RoleIndex role){return goals[role];};
				void setGoal(RoleIndex role, QValue val){goals[role]=val;};
				bool hasGoal(RoleIndex role){return goals[role] != cadiaplayer::play::GOAL_UNKNOWN;};
				QValue getMiniMaxOpti(RoleIndex role);
				QValue getMiniMaxPess(RoleIndex role);
				bool isSolved(RoleIndex role);
				std::string toString(cadiaplayer::play::GameTheory &theory);
			};
			typedef __gnu_cxx::hash_map<MTActionID, MTAction*, cadiaplayer::utils::LongLongHash>	MTActionMap;
			typedef MTActionMap::iterator															MTActionMapItr;
			typedef std::vector<MTActionMap>														MTActions;
			typedef MTActions::iterator																MTActionsItr;
			typedef __gnu_cxx::hash_map<MTActionID, MTNode*, cadiaplayer::utils::LongLongHash>		MTNodeMap;	
			typedef MTNodeMap::iterator																MTNodeMapItr;	
			
			struct MiniMaxValue
			{
				QValue opti;
				QValue pess;
				MiniMaxValue(){
					opti = MM_INIT_OPTI;
					pess = MM_INIT_PESS;
				};
			};
			typedef std::vector<MiniMaxValue> MiniMaxValues;
			struct MTState
			{
				MTStateID id;
				//Depth level;
				Visits n;
				std::vector<Visits> nRAVE;
				MTActions actions;
				MTNodeMap nodes;
				RoleIndex prepared;
				bool expanded;
				Visits comboactions;
				StateFacts facts;
				//QValues minimax;
				MiniMaxValues minimax;
				unsigned int refs;
				std::vector<int> pruned;
				
				MTState(MTStateID sid/*, Depth depth*/, RoleIndex roleCount):id(sid), /*level(depth),*/ n(0), prepared(0), expanded(false), comboactions(0), refs(0)
				{
					actions.resize(roleCount);
					minimax.resize(roleCount);//, MM_UNKNOWN);
					nRAVE.resize(roleCount,0);
					pruned.resize(roleCount,0);
				};
				void reset(MTStateID sid/*, Depth depth*/)
				{
					id=sid;
					//level=depth;
					n=0;
					prepared=0;
					facts.clear();
					for(size_t n = 0 ; n < actions.size() ; n++)
					{
						actions[n].clear();
						minimax[n].opti=MM_INIT_OPTI;
						minimax[n].pess=MM_INIT_PESS;
						nRAVE[n] = 0;
						pruned[n] = 0;
					}
					refs=0;
				}
				void addRef(){refs++;};
				void rmRef(){refs--;};
				bool hasRef(){return refs;};
				bool isExpanded(){return expanded;};
				bool isSolved(cadiaplayer::play::RoleIndex r){return minimax[r].opti == minimax[r].pess;};
				bool isOver(cadiaplayer::play::RoleIndex r){return (minimax[r].pess == GOAL_MAX || minimax[r].opti == GOAL_MIN);};
				bool isWon(cadiaplayer::play::RoleIndex r){return (minimax[r].pess == GOAL_MAX);};
				bool isLost(cadiaplayer::play::RoleIndex r){return (minimax[r].opti == GOAL_MIN);};
				bool allNodesCreated(){return nodes.size() == comboactions;};
				
				
				static MTActionID squashIDs(GameTheory* theory, RoleMoves& moves)
				{
					MTActionID id = 0;
					for(size_t n = 0 ; n < moves.size() ; n++)
					{
						id ^= (theory->getGameStateID(moves[n])>>n);
					}
					return id;
				};
			};
			
			typedef __gnu_cxx::hash_map<MTStateID, MTState*, cadiaplayer::utils::LongLongHash>	MTStateMap;
			typedef MTStateMap::iterator														MTStateMapItr;
			
			typedef std::vector<MTStateMap*>	MTStateMaps;
			typedef std::vector<MTActionBuffer> MTActionBuffers;
			typedef std::list<MTNode*>			MTTrail;
			typedef std::stack<MTState*>		MTStateStorage;
			typedef std::stack<MTAction*>		MTActionStorage;
			typedef std::stack<MTNode*>			MTNodeStorage;
			
			typedef std::vector<MTActionID>		MTPlayMoves;
			
		}
	}
}

#endif //MULTISTATE_H
