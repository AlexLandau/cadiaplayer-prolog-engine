/*
 *  multistack.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 12/9/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef MULTISTACK_H
#define MULTISTACK_H

#include "multitree.h"

// Stack for Depth-First type agents
namespace cadiaplayer{
	namespace play {
		namespace utils {
			
			typedef __gnu_cxx::hash_map<uint32_t, std::vector<int> >	MTNodeIndexMap;
			typedef std::stack<MTNodeIndexMap*> MTNodeIndexStack;
			
			typedef std::stack<MTState*> MTStack;
			
			class MultiStack
			{
			private:
				cadiaplayer::play::GameTheory* m_theory;
				cadiaplayer::play::RoleIndex m_roleCount;
				
				MTActionBuffers	m_buffers;
				MTStateStorage 	m_stateRecycler;
				MTActionStorage m_actionRecycler;
				MTNodeStorage 	m_nodeRecycler;
				
				unsigned int	m_statesCreated;
				unsigned int	m_statesReused;
				unsigned int	m_statesDropped;
				unsigned int	m_actionsCreated;
				unsigned int	m_actionsReused;
				unsigned int	m_actionsDropped;
				unsigned int	m_nodesCreated;
				unsigned int	m_nodesReused;
				unsigned int	m_nodesDropped;
				
				MTStack				m_stack;
				MTNodeIndexStack	m_index;
				bool				m_nodecreated;
			public:
				MultiStack(GameTheory* theory);
				~MultiStack();
				
				MTState* push();
				void pop();
				int size();
				
				MTState* setupState(GameState* state, Depth depth, StateMoves& moves);
				
				// Tree generation
				MTState* 	createState(MTStateID id, Depth depth, GameState* state);
				Move*		createMove(RoleIndex role, Move* move, MTActionID id);
				MTAction* 	createAction(RoleIndex role, MTState* state, Move* move);
				MTAction* 	createAction(RoleIndex role, MTState* state, Move* move, MTActionID hint);
				MTNode* 	createNode(MTState* state, PlayMoves& moves, MTState* child = NULL);		
				//MTNode*		createStateAndNode(RoleIndex role, GameState* state, Move* move, Depth depth, PlayMoves& moves, GameStateID child);
				
			};
		}
	}
}
#endif // MULTISTACK_H

