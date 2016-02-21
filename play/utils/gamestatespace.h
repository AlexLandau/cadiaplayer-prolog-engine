#ifndef GAMESTATESPACE_H
#define GAMESTATESPACE_H

#include <stdlib.h>
#include <vector>
#include <stack>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>
#include <ext/hash_map>
#include "../gametheory.h"

//#define USE_GTQL

#ifdef USE_GTQL
#include "gt_log.h"
struct GTQLNode
{
	unsigned int n;
	double q;
	cadiaplayer::utils::LongLongHashKey move;
	cadiaplayer::utils::LongLongHashKey state;
	unsigned int depth;
};

#endif

//#define LOG_UCT_TREE
#ifdef LOG_UCT_TREE
#include "../../utils/treelog.h"
#endif

namespace cadiaplayer {
	namespace play {
		namespace utils {
			
			
			struct State;
			struct QNode
			{
				unsigned int n;
				double q;
				Move* move;
				State* state;
				double max;
				bool toTerminal;
				unsigned int creationNo;
				QNode(Move* m, State* s, unsigned int cNo)
				{
					n = 0; q = 0.0;move=m;state=s;
					max=0;
					toTerminal=false;
					creationNo=cNo;
				};
			};
			typedef cadiaplayer::utils::LongLongHashKey ZKEY;
			typedef std::vector<ZKEY> ZMAP; 
			typedef __gnu_cxx::hash_map<ZKEY, Move*, cadiaplayer::utils::LongLongHash> ActionBuffer;
			typedef __gnu_cxx::hash_map<ZKEY, QNode*, cadiaplayer::utils::LongLongHash> ActionMap;
			
			struct State
			{
				ZKEY id;
				unsigned int visits;
				ActionMap actions;
				bool prepared;
				State(ZKEY i){id=i;visits=0;prepared=false;};
			};
			
			typedef __gnu_cxx::hash_map<ZKEY, State*, cadiaplayer::utils::LongLongHash> StateMap;
			typedef StateMap::iterator StateMapItr;
			typedef std::vector<StateMap*> StateSpace;
			
			class GameStateSpace
				{
				private:
					//ZMAP keymap;
					GameTheory* m_theory;
					StateSpace m_statespace;
					ActionBuffer m_actionbuffer;
					std::string m_bestActionInfo;
					double m_moveInitQ;
					RoleIndex m_roleindex;
					unsigned int m_creationCounter;
					
				public:
					GameStateSpace(void);
					GameStateSpace(GameTheory* t, RoleIndex ri);
					~GameStateSpace(void);
					void setTheory(GameTheory* t, RoleIndex ri);
					bool isTheorySet(void);
					
					ZKEY getZKey(cadiaplayer::play::parsing::Compound* c);
					ZKEY getZKey(cadiaplayer::play::Move* m);
					ZKEY getZKey(cadiaplayer::play::GameState* s);
					void prepareState(cadiaplayer::play::GameState* s, RoleMoves& moves, unsigned int depth);
					QNode* updateValue(QNode* node, double reward);
					QNode* updateValue(cadiaplayer::play::GameState* s, cadiaplayer::play::Move* move, unsigned int depth, double reward);
					Move* bestAction(cadiaplayer::play::GameState* s, unsigned int depth, double& qValue);
					Move* maxAction(cadiaplayer::play::GameState* s, unsigned int depth, double& qValue);
					double maxQValue(GameState* s, unsigned int depth);
					double maxQValue(QNode* n);
					double maxQValue(State* s);
					QNode* getNode(cadiaplayer::play::GameState* s, Move* move, unsigned int depth);
					void dropDepth(unsigned int depth);
					void drop();
					void reset();
					void getMoves(GameTheory& theory, RoleIndex role, unsigned int depth, RoleMoves& moves);
					Move* getBufferedMove(Move* move);
					//Move* getBufferedMove(cadiaplayer::play::parsing::Compound* compound);
					void setMoveInitQ(double q){m_moveInitQ = q;};
					double getMoveInitQ(){return m_moveInitQ;};
					StateMap* getStateMap(unsigned int depth){return m_statespace[depth];};
					
					// Info functions.
					std::string getBestActionInfo(void){return m_bestActionInfo;};
					void setBestActionInfo(std::string bestActionInfo){m_bestActionInfo=bestActionInfo;};
					std::string generateActionInfo(GameState* s, unsigned int depth);
					std::string getStats(void);
					std::string getDetails(void);
					std::string getActionBufferInfo(void);
					std::string stateActionSizeInfo(void);
					std::string actionBufferSizeInfo(void);
					State* lookupState(unsigned int depth, ZKEY key);
					QNode* lookupAction(State* state, ZKEY key);
					
					static ZKEY generateZKey(void)
					{
						return (((ZKEY)rand()) << 32) | rand();
						
					}
#ifdef USE_GTQL
					void logGTQL(std::string id)
					{
						GTDataDescript gtDataDescr = 
						{ 
							"CadiaPlayer", sizeof(GTQLNode), 0, {}, 5,
							{ 
								{ "state", offsetof(GTQLNode, state), sizeof(cadiaplayer::utils::LongLongHashKey)},
								{ "depth", offsetof(GTQLNode, depth), sizeof(unsigned int)},
								{ "move", offsetof(GTQLNode, move), sizeof(cadiaplayer::utils::LongLongHashKey)},
								{ "n", offsetof(GTQLNode, n), sizeof(unsigned int)},
								{ "q", offsetof(GTQLNode, q), sizeof(double)}
							}
						};
						GTLogHdl hGTL = gtl_newHdl("CadiaPlayer", &gtDataDescr);
						if(hGTL == NULL) return;
						std::string filename = "CadiaPlayer_";
						filename += id;
						filename += ".gtql";
						std::string strFEN = "" + getZKey(m_theory->getStateRef());
						gtl_startTree(hGTL, filename.c_str(), strFEN.c_str());//GameTheory::listToString(m_theory->getStateRef(), m_theory->getSymbols()).c_str());
						size_t depth = 0;
						for(StateSpace::iterator dit = m_statespace.begin() ; dit != m_statespace.end() ; dit++)
						{
							for(StateMap::iterator sit = (*dit)->begin() ; sit != (*dit)->end() ; sit++)
							{
								for(ActionMap::iterator ait = (*sit).second->actions.begin() ; ait != (*sit).second->actions.end() ; ait++)
								{
									GTQLNode data;
									gtl_enterNode(hGTL, cadiaplayer::utils::LongLongHash::hash(ait->first));
									data.state = sit->first;
									data.depth = depth;
									data.move = ait->first;
									data.n = ait->second->n;
									data.q = ait->second->q;
									gtl_exitNode(hGTL, &data);
								}
							}
							depth++;
						}
						
						gtl_stopTree(hGTL);
						gtl_deleteHdl(&hGTL);
					};
#endif
				private:
					State* lookupAndCreateState(unsigned int depth, ZKEY key, GameState* s);
					QNode* lookupAndCreateAction(State* state, Move* move, ZKEY key);
					void buildStateSpace(void);			
					
#ifdef LOG_UCT_TREE
				public:
					cadiaplayer::utils::TreeLog m_treelog;
#endif
				};
		}}} // namespaces
#endif // GAMESTATESPACE_H
