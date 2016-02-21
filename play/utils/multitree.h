#ifndef MULTITREE_H
#define MULTITREE_H

#include "multistate.h"

namespace cadiaplayer{
	namespace play {
		namespace utils {
			
			class MoveSelectionData;
			
			class MultiTree
			{
			private:
				GameTheory* 	m_theory;
				MTStateMaps 	m_tree;
				MTStateMap		m_transpositions;
				MTActionBuffers	m_buffers;
				MTTrail*		m_trail;
				MTStateStorage 	m_stateRecycler;
				MTActionStorage m_actionRecycler;
				MTNodeStorage 	m_nodeRecycler;
				std::string 	m_bestActionInfo;
				QValue			m_moveInitQ;
				MTState*		m_nodeAdded;
				RoleIndex		m_roleCount;
				Depth			m_currentDepth;
				bool			m_muteretract;
				
				unsigned int	m_statesCreated;
				unsigned int	m_statesReused;
				unsigned int	m_statesDropped;
				unsigned int	m_actionsCreated;
				unsigned int	m_actionsReused;
				unsigned int	m_actionsDropped;
				unsigned int	m_nodesCreated;
				unsigned int	m_nodesReused;
				unsigned int	m_nodesDropped;
				std::string 	m_memoryInfo;
				
			public:
				MultiTree(GameTheory* theory);
				~MultiTree();

				MTActionBuffer& getActionBuffer(RoleIndex role);
				
				MTState* 	prepareState(RoleIndex role, MTStateID id, RoleMoves& moves, Depth depth, GameState* state);
				MTState* 	prepareState(RoleIndex role, MTStateID id, RoleMoves& moves, Depth depth, GameState* state, unsigned int& counter);
				MTAction* 	updateValue(RoleIndex role, GameState* state, Move* move, Depth depth, QValue reward, PlayMoves& moves, GameStateID child);
				MTAction* 	updateValue(RoleIndex role, MTNode* node, QValue reward);
				MTAction*	updateValue(RoleIndex role, MTState* state, Depth depth, QValue reward, PlayMoves& moves, GameStateID child);
				MTAction* 	updateMMValue(RoleIndex role, GameState* state, Move* move, Depth depth, QValue reward, PlayMoves& moves, GameStateID child);
				MTAction* 	updateMMValue(RoleIndex role, MTNode* node, QValue reward);
				MTAction* 	updateValueRAVE(RoleIndex role, MTNode* node, QValue reward);
				MTAction* 	updateValueRAVESibling(RoleIndex role, MTAction* action, QValue reward);
				
				MTAction* 	updateValueAndWaypoint(RoleIndex role, GameState* state, Move* move, Depth depth, QValue reward, PlayMoves& moves, GameStateID child);
				MTAction* 	updateValueAndWaypoint(RoleIndex role, MTNode* node, QValue reward);
				void		updateWaypoint(MTAction* action);
				
				void		getSolverValue(MTNode* node, RoleIndex role, MiniMaxValue& value);
				bool		updateSolverValue(MTNode* node, Depth depth);
				
				Move* 		bestAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		controlAction(RoleIndex role, MTState* s, QValue& qValue, bool info = false);
				Move* 		maxAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		mostExploredAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		goalAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		mmAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		bestRAVEAction(RoleIndex role, MTState* s, QValue& qValue);
				Move* 		controlRAVEAction(RoleIndex role, MTState* s, QValue& qValue, bool info = false);
				double		bestRAVE(MTAction* node);
				
				void		bestActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings);
				void		maxActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings);
				void		controlActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings);
				void		bestRAVEActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings);
				void		controlRAVEActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings);
				
				
				void 		dropDepth(Depth depth);
				void 		drop();
				void 		destroyDepth(Depth depth);
				void 		destroy();
				void 		reset();
				std::size_t size(){return m_tree.size();};
				
				void		newRound(Depth currentDepth){m_currentDepth=currentDepth;};
				MTNode*		makeAction(MTStateID sid, Depth depth, PlayMoves& moves);
				void		retractAction();
				void		muteRetract();
				void		syncRetract();
				bool 		getMoves(RoleIndex role, MTStateID id, Depth depth, RoleMoves& moves);
				bool		isTerminal();
				QValue		goal(RoleIndex role);
				QValue		goal(MTNode* node, RoleIndex role);
				Move* 		getBufferedMove(RoleIndex role, Move* move);
				Move* 		getBufferedMove(RoleIndex role, MTActionID id);
				void 		setMoveInitQ(QValue q){m_moveInitQ = q;};
				QValue 		getMoveInitQ(){return m_moveInitQ;};
				MTStateMap* getStateMap(Depth depth){return m_tree[depth];};
				
				// Trails handling
				MTNode*		pushTrail(MTStateID id, Depth depth, PlayMoves& moves);
				MTNode* 	pushTrail(MTNode* node);
				MTNode*		popTrail();
				MTNode* 	peekTrail();
				MTNode* 	peekTrailParent();
				std::size_t	trailSize(){return m_trail->size();};
				void		terminateTrail(MTStateID id, Depth depth);
				bool 		trailEmpty(){return m_trail->empty();};
				void		resetTrail();
				MTTrail*	hijackTrail();
				
				// Info functions.
				void		loadMoveInfo     (RoleIndex role, MTNode* n, MTAction* a, std::stringstream& ss);
				void		loadMoveMaxInfo  (RoleIndex role, MTNode* n, MTAction* a, std::stringstream& ss);
				void		loadMoveGoalInfo (RoleIndex role, MTNode* a, std::stringstream& ss);
				void		loadMoveMMInfo   (RoleIndex role, MTAction* a, MTAction* b, std::stringstream& ss, QValue opti, QValue pess, QValue mmval, int t);
				void		loadRAVEMoveInfo (RoleIndex role, MTNode* n, MTAction* a, QValue rave, std::stringstream& ss);
				std::string getBestActionInfo(void){return m_bestActionInfo;};
				void		setBestActionInfo(std::string bestActionInfo){m_bestActionInfo=bestActionInfo;};
				std::string getMemoryInfo(void);
				void		resetMemoryInfo(void);
				void		dumpToFile(std::string filename, GameState* s, Depth d);
				void		dumpState(MTState* state, std::ofstream& file, std::queue<MTState*>* children);
				void		dumpActionBuffer(RoleIndex role, std::stringstream& ss);
				
				// Data lookup
				MTState* 	lookupState(MTStateID id, Depth depth);
				MTState* 	lookupState(GameState* s, Depth depth);
				Move*		lookupMove(RoleIndex role, MTActionID id);
				MTAction*	lookupAction(RoleIndex role, MTState* state, MTActionID id);
				MTAction* 	lookupAction(RoleIndex role, MTState* state, Move* move);
				MTAction*	lookupAction(RoleIndex role, GameState* state, Move* move, Depth depth);
				MTNode*		lookupNode(MTState* state, PlayMoves& moves);				
				MTNode*		lookupSingleNode(MTState* state, Move* move);
				
				// Tree generation
				MTState* 	createState(MTStateID id, Depth depth, GameState* state);
				Move*		createMove(RoleIndex role, Move* move, MTActionID id);
				MTAction* 	createAction(RoleIndex role, MTState* state, Move* move);
				MTAction* 	createAction(RoleIndex role, MTState* state, Move* move, MTActionID hint);
				MTNode* 	createNode(MTState* state, PlayMoves& moves, MTState* child = NULL);		
				MTNode*		createStateAndNode(RoleIndex role, GameState* state, Move* move, Depth depth, PlayMoves& moves, GameStateID child);
				
				// Log Tree
				void		writeTreeToFile(std::string filename, GameState* s);
				void		traverseTreeForWriting(MTStateMap& transp, MTState* state, std::ofstream& file);
				void		writeStateToFile(MTState* state, std::ofstream& file, std::queue<MTState*>* children);
				
				// Move Trails
				MTState*	setupState(GameState* state, Depth depth, AllRoleMoves& moves);
				MTState*	setupState(GameState* state, Depth depth, RoleIndex role, RoleMoves& moves);
				bool		verifyTransition(MTState* state, MTPlayMoves& moves, PlayMoves& pm);
				Move*		verifyMove(MTState* state, GameStateID move, cadiaplayer::play::RoleIndex role);
				//				bool		expandPath(cadiaplayer::play::utils::MTStateID sid, cadiaplayer::play::utils::Depth depth, cadiaplayer::play::utils::MoveSelectionData path);
				std::string	pathToString(cadiaplayer::play::utils::MoveSelectionData& path);
				
			private:
				//MTActionID	squashIDs(RoleMoves& moves);	
			};
		}
	}
}

#endif //MULTITREE_H
