/*
 *  multistack.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 12/9/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "multistack.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;

MultiStack::MultiStack(cadiaplayer::play::GameTheory* theory)
{
	m_theory = theory;
	m_roleCount = theory->getRoles()->size();
	m_buffers.resize(m_roleCount);
}

MultiStack::~MultiStack()
{
	
	while (!m_stack.empty()) 
	{
		pop();
	}
	
	//size_t counter = 0;
	while(!m_nodeRecycler.empty())
	{
		//	counter++;
		delete m_nodeRecycler.top();
		m_nodeRecycler.pop();
	}
	//std::cerr << "Deleted " << counter << " nodes" << std::endl;
	//counter = 0;
	while(!m_actionRecycler.empty())
	{
		//	counter++;
		delete m_actionRecycler.top();
		m_actionRecycler.pop();
	}
	//std::cerr << "Deleted " << counter << " actions" << std::endl;
	//counter = 0;
	while(!m_stateRecycler.empty())
	{
		//	counter++;
		delete m_stateRecycler.top();
		m_stateRecycler.pop();
	}
	//std::cerr << "Deleted " << counter << " states" << std::endl;
	MTActionBufferItr del;
	for(size_t n = 0 ; n < m_buffers.size() ; n++)
	{
		for(del = m_buffers[n].begin() ; del != m_buffers[n].end() ; ++del)
		{
			delete (*del).second;
		}
		m_buffers[n].clear();
	}
}

MTState* MultiStack::push()
{
	int d = m_stack.size();
	StateMoves moves;
	m_theory->getMoves(moves);
	m_stack.push(setupState(m_theory->getStateRef(), d, moves));
	return m_stack.top();
}

void MultiStack::pop()
{
	MTState* state = m_stack.top();
	
	for(MTActionsItr map = state->actions.begin() ; map != state->actions.end() ; map++)
	{
		for(MTActionMapItr action = map->begin() ; action != map->end() ; action++)
		{
			MTAction* a = action->second;
			m_actionRecycler.push(a);
			m_actionsDropped++;
		}
		map->clear();
	}
	for(MTNodeMapItr node = state->nodes.begin() ; node != state->nodes.end() ; node++)
	{
		node->second->reset();
		m_nodeRecycler.push(node->second);
		m_nodesDropped++;
	}
	state->nodes.clear();
	m_stateRecycler.push(state);
	m_statesDropped++;
	
	m_stack.pop();
	
	/*std::cerr << "deleting indexes\n";
	MTNodeIndexMap::iterator itr = m_index.top()->begin();
	for( ; itr != m_index.top()->end() ; ++itr)
	{
		for(int n = 0 ; n < itr->second.size() ; n++)
		{
			std::cerr << itr->second[n] << " ";
		}
		std::cerr << std::endl;
	}
	std::cerr << ".\n";
	delete m_index.top();
	m_index.pop();*/
}

int MultiStack::size()
{
	return m_stack.size();
}

MTState* MultiStack::setupState(GameState* state, Depth depth, StateMoves& moves)
{
	MTStateID sid = m_theory->getGameStateID(state); 
	MTState* s = createState(sid, depth, state);
	if(s->prepared == m_roleCount)
		return s;
	
	MoveCombinations mc;
	std::vector<Move*> comb;
	s->comboactions = 0;
	MTNode* node;
	MTNodeIndexMap* map = new MTNodeIndexMap();
	m_index.push(map);
	for(mc.begin(&moves) ; !mc.end() ; ++mc)
	{
		comb.clear();
		mc.get(comb);
		s->comboactions++;
		node = createNode(s, comb, NULL);
		
		/*if(!m_nodecreated)
			continue;
		
		uint32_t x = reinterpret_cast<uint32_t> (node);
		std::cerr << "x = " << x << "      " << node->actions[0]->id << " " << node->actions[1]->id << std::endl;
		std::cerr << node->actions[0]->move->toString(m_theory->getSymbols()) << " " << node->actions[1]->move->toString(m_theory->getSymbols()) << std::endl;
		std::vector<int>& ind = (*map)[x];
		mc.getIndex(ind);
		std::cerr << "index set to\n";
		for(int n = 0 ; n < ind.size() ; n++)
		{
			std::cerr << ind[n] << " ";
		}
		std::cerr << "\n.\n";*/
	}
	
	return s;
}

MTState* MultiStack::createState(MTStateID id, Depth depth, GameState* state)
{
	MTState* sp;
	if(m_stateRecycler.size())
	{
		sp = m_stateRecycler.top();
		sp->reset(id/*, depth*/);
		m_stateRecycler.pop();
		m_statesReused++;
	}
	else
	{
		sp = new MTState(id/*, depth*/, m_roleCount);
		m_statesCreated++;
	}
	m_theory->getStateFacts(*state, sp->facts);
	return sp;	
}
Move* MultiStack::createMove(RoleIndex role, Move* move, MTActionID id)
{
	MTActionBufferItr itr = m_buffers[role].find(id);
	if(itr == m_buffers[role].end()) 
	{
		Move* mp = new Move(*move);
		m_buffers[role][id] = mp;
		return mp;
	}
	return itr->second;
}
MTAction* MultiStack::createAction(RoleIndex role, MTState* state, Move* move)
{
	MTActionID id = m_theory->getGameStateID(move);
	return createAction(role, state, move, id);
}
MTAction* MultiStack::createAction(RoleIndex role, MTState* state, Move* move, MTActionID hint)
{
	MTActionMap& map = state->actions[role];
	MTActionMapItr itr = map.find(hint);
	Move* buffered = createMove(role, move, hint);
	if(itr == map.end())
	{
		MTAction* ap;
		if(m_actionRecycler.size())
		{
			ap = m_actionRecycler.top();
			ap->reset(hint, state, buffered);
			m_actionRecycler.pop();
			m_actionsReused++;
		}
		else
		{
			ap = new MTAction(hint, state, buffered);
			m_actionsCreated++;
		}
		map[hint] = ap;
		return ap;
	}
	return itr->second;
}
MTNode* MultiStack::createNode(MTState* state, PlayMoves& moves, MTState* child)
{
	MTActionID combo = MTState::squashIDs(m_theory, moves);	
	MTNodeMapItr itr = state->nodes.find(combo);
	if(itr == state->nodes.end())
	{
		MTNode* np;
		if(m_nodeRecycler.size())
		{
			np = m_nodeRecycler.top();
			m_nodeRecycler.pop();
			m_nodesReused++;
		}
		else
		{
			np = new MTNode();
			m_nodesCreated++;
		}
		np->parent = state;
		np->child = child;
		state->nodes[combo] = np;
		MTAction* a;
		for(size_t n = 0 ; n < moves.size() ; n++)
		{
			a = createAction(n, state, moves[n]);
			np->actions.push_back(a);
			if(state->comboactions == state->actions[n].size())
				a->node = np;
		}
		np->goals.resize(moves.size(), cadiaplayer::play::GOAL_UNKNOWN);
		m_nodecreated = true;
		return np;
	}
	m_nodecreated = false;
	return itr->second;
}

/*MTNode* MultiStack::createStateAndNode(RoleIndex role, GameState* state, Move* move, Depth depth, PlayMoves& moves, GameStateID child)
{
	MTState* s = createState(m_theory->getGameStateID(state), depth, state);
	if(s->prepared != m_roleCount)
	{		
		Move* m = NULL;
		MTActionID aid = 0; 
		std::cerr << "createState moves begin" << std::endl;
		for(size_t n = 0 ; n < moves.size() ; ++n)
		{
			std::cerr << moves[n]->toString(m_theory->getSymbols()) << std::endl;
			aid = m_theory->getGameStateID(moves[n]);
			m = createMove(n, moves[n], aid);
			createAction(n, s, m, aid); 
		}
		for( RoleIndex i = 0 ; i < m_roleCount ; ++i)
		{
			std::cerr << "Role " << (i+1) << std::endl;
			for(MTActionMapItr itr = s->actions[i].begin() ; itr != s->actions[i].end() ; ++itr)
			{
				std::cerr << itr->second->move->toString(m_theory->getSymbols()) << std::endl;
			}
		}
		std::cerr << "createState moves end" << std::endl;
		
	}
	return createNode(s, moves, NULL);
}*/
