#include "multitree.h"
#include "movetrails.h"

//#define DEBUG_MULTITREE

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;

MultiTree::MultiTree(GameTheory* theory):
m_theory(theory),
m_moveInitQ(cadiaplayer::play::GOAL_DRAW),
m_nodeAdded(NULL),
m_roleCount(m_theory->getRoles()->size()),
m_trail(new MTTrail()),
m_currentDepth(0),
m_muteretract(false)
{
	m_buffers.resize(m_roleCount);
}
MultiTree::~MultiTree()
{
	//std::cerr << "Start of Multitree destructor\n";
	destroy();
	m_tree.clear();
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
	
	if(m_trail != NULL)
		delete m_trail;
	
	//std::cerr << "End of Multitree destructor\n";
}

MTActionBuffer& MultiTree::getActionBuffer(RoleIndex role)
{
	return m_buffers[role];
}

MTState* MultiTree::prepareState(RoleIndex role, MTStateID id, RoleMoves& moves, Depth depth, GameState* state)
{
	if(m_nodeAdded != NULL)
		return NULL;
	MTState* s = createState(id, depth, state);
	if(s->prepared == m_roleCount)
		return NULL;
	
	//if(s->prepared < m_roleCount && s->actions[role].size())
	//{
	//	std::cerr << "Prepare called on state with " << s->actions[role].size() << " actions " << s->actions[role].begin()->second->move->toString(m_theory->getSymbols()) << " for role " << role << " at depth " << depth << "\n";
	//}
	//else 
	//{
	//	std::cerr << "Called prep on empth state\n";
	//}

	
	s->prepared++;
	MTActionID aid = 0; 
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		aid = m_theory->getGameStateID(moves[n]);
		Move* move = createMove(role, moves[n], aid);
		createAction(role, s, move, aid); 
	}
	if(s->prepared == m_roleCount)
	{
		m_nodeAdded = s;	
		s->comboactions = 1;
		for(Visits n = 0 ; n < m_roleCount ; n++)
		{
			s->comboactions *= s->actions[n].size();
		}
	}
	/*if(s->prepared == 2 && s->comboactions == 0)
	{
		std::cerr << "comboactions is 0\n";
		std::cerr << "prepared is " << s->prepared << "\n";
		std::cerr << "actions[0] is " << s->actions[0].size() << "    actions[1] is " << s->actions[1].size();
		std::cerr << GameTheory::listToString(state, m_theory->getSymbols());
		std::cerr << "\n";
	}*/
	return s;
}
MTState* MultiTree::prepareState(RoleIndex role, MTStateID id, RoleMoves& moves, Depth depth, GameState* state, unsigned int& counter)
{
	//std::cerr << "Counter prepare (" << counter << ")\n";
	if(counter)
	{
		MTState* s = prepareState(role, id, moves, depth, state);
		if(s)
		{
			if(role == m_roleCount-1) 
				--counter;
		}
		//std::cerr << "Counter : " << counter << std::endl;
		m_nodeAdded = NULL;
		return counter ? NULL : s;
	}
	//std::cerr << "Counter depleted" << std::endl;
	return prepareState(role, id, moves, depth, state);
}
MTAction* MultiTree::updateValue(RoleIndex role, GameState* state, Move* move, Depth depth, QValue reward, PlayMoves& moves, GameStateID child)
{
	MTNode* node = createStateAndNode(role, state, move, depth, moves, child);
	return updateValue(role, node, reward);
}
MTAction* MultiTree::updateValue(RoleIndex role, MTState* state, Depth depth, QValue reward, PlayMoves& moves, GameStateID child)
{
	MTNode* node = createNode(state, moves, lookupState(child, depth+1));
	return updateValue(role, node, reward);
}

MTAction* MultiTree::updateValue(RoleIndex role, MTNode* node, QValue reward)
{
	// Update the Q node incrementally :
	// NewEstimate <- OldEstimate + StepSize[Target - OldEstimate]
	
	// Note down this visit only for the first role so not to multiply it up.
	MTState* state = node->parent;
	if(!role)
		state->n++;
	MTAction* action = node->actions[role];
	action->n++;
	
#ifdef USE_MOVING_AVERAGES
	if(action->n > MOVING_AVERAGES_THRESHOLD)
		action->q = action->q + MOVING_AVERAGES_STEPSIZE*(reward - action->q);
	else
		action->q = action->q + (1.0/action->n)*(reward - action->q);
#else
	action->q = action->q + (1.0/action->n)*(reward - action->q);
#endif
	if(action->max < reward)
		action->max=reward;
	
	if(!node->expanded && (node->isTerminal() || (node->child && node->child->expanded)))
		node->expanded = true;
	
	return action;
}

MTAction* MultiTree::updateMMValue(RoleIndex role, GameState* state, Move* move, Depth depth, QValue reward, PlayMoves& moves, GameStateID child)
{
	MTState* s = createState(m_theory->getGameStateID(state), depth, state);
	Move* m = NULL;
	MTActionID aid = 0; 
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		aid = m_theory->getGameStateID(moves[n]);
		m = createMove(role, moves[n], aid);
		createAction(role, s, m, aid); 
	}
	MTNode* node = createNode(s, moves, lookupState(child, depth+1));
	return updateMMValue(role, node, reward);
}
MTAction* MultiTree::updateMMValue(RoleIndex role, MTNode* node, QValue reward)
{
	MTAction* action = node->actions[role];
	// Update the Q node incrementally :
	// NewEstimate <- OldEstimate + StepSize[Target - OldEstimate]
	
	// Note down this visit only for the first role so not to multiply it up.
	if(!role)
		node->parent->n++;
	action->n++;
	action->q = action->q + (1.0/action->n)*(reward - action->q);
	if(action->max < reward)
		action->max=reward;

	MTState* parent = node->parent;
	bool me = true;
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		if(r == role)
			continue;
		if(parent->actions[r].size() > 1)
		{
			me = false;
			break;
		}
	}
	/*QValue mm = MM_UNKNOWN;
	if(parent->minimax[role] == MM_UNKNOWN)
	{
		if(node->isTerminal())
		{
			if(me && node->getGoal(role) == 100)
			{
				parent->minimax[role] = 100;
				action->pruned = true;
				//action->q = 100;
				return action;
			}
			else if(!me && node->getGoal(role) == 0)
			{
				node->parent->minimax[role] = 0;
				action->pruned = true;
				//action->q = 0;
				return action;
			}
		}
		if(parent->nodes.size() == parent->actions[0].size()*parent->actions[1].size())
		{
			bool ok = true;
			MTNodeMapItr itr;
			MTNode* n = NULL;
			if(me)
			{
				for(itr = parent->nodes.begin() ; itr != parent->nodes.end() ; itr++)
				{
					n = itr->second;
					if(n->isTerminal())
					{
						if(mm < n->getGoal(role))
							mm = n->getGoal(role);
						continue;
					}
					if(n->child != NULL && n->child->minimax[role] != MM_UNKNOWN)
					{
						if(mm < n->child->minimax[role])
							mm = n->child->minimax[role];
						continue;
					}
					if(mm==100)
					{
						ok = true;
						break;
					}
					ok = false;
				}	
			}
			else
			{
				mm = 100;
				for(itr = parent->nodes.begin() ; itr != parent->nodes.end() ; itr++)
				{
					n = itr->second;
					if(n->isTerminal())
					{
						if(mm > n->getGoal(role))
							mm = n->getGoal(role);
						continue;
					}
					if(n->child != NULL && n->child->minimax[role] != MM_UNKNOWN)
					{
						if(mm > n->child->minimax[role])
							mm = n->child->minimax[role];
						continue;
					}
					if(mm==0)
					{
						ok = true;
						break;
					}
					ok = false;
				}
			}
			if(ok)
				parent->minimax[role] = mm;
		}
	}	
	else
	{
		if(node->isTerminal() && node->child && node->getMiniMax(role) == MM_UNKNOWN)
			node->child->minimax[role] = parent->minimax[role];
	}
	mm = parent->minimax[role];	
	if(me && mm != MM_UNKNOWN)
	{
		action->pruned = true;
		//action->q = mm;
		node->makeTerminal();
		node->setGoal(role, mm);
	}*/
		
	return action;
}

//bool propagatePessimistic(MTState* )
//{
/*
	Algorithm 1 Pseudo-code for propagating pessimistic bounds
	procedure prop-pess 
	arguments node s 
	if s is not the root node then
		Let n be the parent of s 
		Let old_pess := pess(n) 
		if old_pess < pess(s) then
			if n is a Max node then 
				pess(n) := pess(s) 
				prop-pess(n)
			else
				pess(n) := mins′∈children(n) pess(s′) 
				if old_pess > pess(n) then
					prop-pess(n) 
				end if
			end if 
		end if
	end if
 */ 

//}
//bool propagateOptimistic()
//{
/*	
 Algorithm 2 Pseudo-code for propagating optimistic bounds
 procedure prop-opti 
 arguments node s 
 if s is not the root node then
 Let n be the parent of s 
 Let old_opti := opti(n) 
 if old_opti > opti(s) then
 if n is a Max node then 
 opti(n) := maxs′∈children(n) opti(s′) 
 if old_opti > opti(n) then
 prop-opti(n) 
 end if
 else
 opti(n) := opti(s)
 prop-opti(n) 
 end if
 end if 
 end if
 */
//}
void MultiTree::getSolverValue(MTNode* node, RoleIndex role, MiniMaxValue& value)
{
	if(node->isTerminal())
	{
		value.opti = node->goals[role];
		value.pess = node->goals[role];
		return;
	}
	if(!node->child)
	{
		value.opti = MM_INIT_OPTI;
		value.pess = MM_INIT_PESS;
		return;
	}
	value.opti = node->getMiniMaxOpti(role);
	value.pess = node->getMiniMaxPess(role);
}

//#define DEBUG_MCTS_SOLVER

bool MultiTree::updateSolverValue(MTNode* node, Depth depth)
{
#ifdef DEBUG_MCTS_SOLVER
	std::stringstream serr;
	if(!node)
		std::cerr << "Calling update for null node, aborting update...\n"; 
#endif
	if(!node)
		return false;
	MTState* state = node->parent;
#ifdef DEBUG_MCTS_SOLVER
	//std::stringstream serr;
	if(!state)
		std::cerr << "Calling update for node with no parent, aborting update...\n"; 
#endif	
	if(!state)
		return false;
	if(node->child)
	{
		//std::cerr << "Solver lookup:";
		//MTState* x = 
		lookupState(node->child->id, depth+1);
		//if(!x)
		//	std::cerr << "fail\n";
	}
#ifdef DEBUG_MCTS_SOLVER
	serr << "Calling update for parent of " 
	<< node->actions[0]->move->compound.toString(m_theory->getSymbols()) << " - "
	<< node->actions[1]->move->compound.toString(m_theory->getSymbols()) << std::endl;
#endif
	RoleIndex play = 0;
	RoleIndex noop = 1;
	if(state->actions[play].size() == 1)
	{
		if(state->actions[noop].size() == 1)
		{
			MTActions& acts = state->actions;
			MTActionMap amap = acts[noop];
			MTAction* act = amap.begin()->second; 
			Move* m = act->move;
			if(m->compound.getArguments()->size() > 1)
			{
				play = 1;
				noop = 0;
			}
		}
		else
		{
			play = 1;
			noop = 0;
		}
	}
	MiniMaxValue& max = state->minimax[play];
	if(max.opti == max.pess)
		return false;
#ifdef DEBUG_MCTS_SOLVER	
	//std::stringstream serr;
	serr << "Solver-Update on state (" << state->id << ")\n"
	<< GameTheory::listToString(m_theory->getStateRef(), m_theory->getSymbols())
	<< "With max as " << (*m_theory->getRoles())[play] 
	<< " and min as " << (*m_theory->getRoles())[noop] << std::endl
	<< "Playing "
	<< node->actions[play]->move->compound.toString(m_theory->getSymbols()) << " - "
	<< node->actions[noop]->move->compound.toString(m_theory->getSymbols())
	<< std::endl;
#endif
	MiniMaxValue& min = state->minimax[noop];
	MiniMaxValue childMax;
	getSolverValue(node, play, childMax);
	MiniMaxValue childMin;
	getSolverValue(node, noop, childMin);
	
	bool update = false;
	MTNode* temp;
	
	if(max.pess < childMax.pess)
	{
#ifdef DEBUG_MCTS_SOLVER
		serr	<< "Entering pess update because max.pess < childMax.pess is " 
					<< max.pess << " < " << childMax.pess << std::endl;
		
		serr << "State has " << state->nodes.size() << " nodes with " 
		<< state->actions[play].size() << " max actions and " << state->actions[noop].size() << " min actions"
		<< " giving comboactions of count " << state->comboactions
		<< std::endl;
		
		for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
		{
			temp = nitr->second;
			serr << "Probing move node of " 
			<< temp->actions[play]->move->compound.toString(m_theory->getSymbols()) << " - "
			<< temp->actions[noop]->move->compound.toString(m_theory->getSymbols()) << std::endl;
		}

		serr << "max.pess : " << max.pess << " -> " << childMax.pess << std::endl;
		serr << "min.opti : " << min.opti << " -> " << childMin.opti << std::endl;
#endif		
		max.pess = childMax.pess;
		min.opti = childMin.opti;
		update = true;
	}
	else if(childMax.opti < max.opti && state->nodes.size() >= state->comboactions)
	{
#ifdef DEBUG_MCTS_SOLVER
		serr	<< "Entering opti update because childMax.opti < max.opti id " 
					<< childMax.opti << " < " << max.opti << std::endl;
#endif
		double maxOpti = childMax.opti;
		double minPess = childMin.pess;
#ifdef DEBUG_MCTS_SOLVER
		serr << "State has " << state->nodes.size() << " nodes with " 
		<< state->actions[play].size() << " max actions and " << state->actions[noop].size() << " min actions"
		<< " giving comboactions of count " << state->comboactions
		<< std::endl;
#endif
		for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
		{
			temp = nitr->second;
#ifdef DEBUG_MCTS_SOLVER
			serr << "Scanning move node of " 
			<< temp->actions[play]->move->compound.toString(m_theory->getSymbols()) << " - "
			<< temp->actions[noop]->move->compound.toString(m_theory->getSymbols()) << std::endl;
#endif
			if(temp->child)
				lookupState(temp->child->id, depth+1);
			getSolverValue(temp, play, childMax);
			getSolverValue(temp, noop, childMin);
			if(maxOpti < childMax.opti)
			{	
#ifdef DEBUG_MCTS_SOLVER
				serr	<< "Node adjustment detected because maxOpti < childMax.opti is " 
							<< maxOpti << " < " << childMax.opti << std::endl;
#endif
				maxOpti = childMax.opti;
				minPess = childMin.pess;
				if(maxOpti == max.opti)
				{
#ifdef DEBUG_MCTS_SOLVER
					serr << "Early halt on node scanning because maxOpti == max.opti is : " 
					<< maxOpti << " == " << max.opti << std::endl;
#endif
					break;
				}
			}
		}
#ifdef DEBUG_MCTS_SOLVER
		serr << "max.opti : " << max.opti << " -> " << maxOpti << std::endl;
		serr << "min.pess : " << min.pess << " -> " << minPess << std::endl;
#endif
		max.opti = maxOpti;
		min.pess = minPess;
		update = true;
	}
#ifdef DEBUG_MCTS_SOLVER	
	if(state->isSolved(play))
		serr << "Max solved to " << max.pess << "/" << max.opti << std::endl;
	if(state->isSolved(noop))
		serr << "Min solved to " << min.pess << "/" << min.opti << std::endl;
	if(update)
	{
		serr << "Solver-Update on state result : TRUE\n";
		std::cerr << serr.str();
	}
//	else 
//		std::cerr << "Solver-Update on state result : FALSE\n"
//		<< "Max: " << node->actions[play]->move->compound.toString(m_theory->getSymbols()) << " - "
//		<< "Min: " << node->actions[noop]->move->compound.toString(m_theory->getSymbols()) << std::endl;
#endif	
	
	return update;
}

MTAction* MultiTree::updateValueRAVE(RoleIndex role, MTNode* node, QValue reward)
{
	// Update the RAVE Q node incrementally :
	// NewEstimate <- OldEstimate + StepSize[Target - OldEstimate]
	
	MTAction* action = node->actions[role];
	action->state->nRAVE[role]++;
	action->nRAVE++;
	action->qRAVE = action->qRAVE + (1.0/action->nRAVE)*(reward - action->qRAVE);
	
	return action;
}
MTAction* MultiTree::updateValueRAVESibling(RoleIndex role, MTAction* action, QValue reward)
{
	// Update the Q node incrementally :
	// NewEstimate <- OldEstimate + StepSize[Target - OldEstimate]
	
	action->state->nRAVE[role]++;
	action->nRAVE++;
	action->qRAVE = action->qRAVE + (1.0/action->nRAVE)*(reward - action->qRAVE);
	return action;
}

Move* MultiTree::bestAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	ss << "State visits : " << state->n << "   MiniMax : [ ";
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
	}
	ss << "]" << std::endl;
	Move* move = NULL;
	QValue curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		if(first)
		{
			curQ = a->second->q;
			curA = a->first;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == a->second->q)
			tiebreak.push_back(a->first);
		else if(curQ < a->second->q)
		{
			curQ = a->second->q;
			curA = a->first;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadMoveInfo(role, NULL, a->second, ss);
		ss << std::endl;
	}
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	
	//dumpActionBuffer(role, ss);
	
	m_bestActionInfo = ss.str();
	return move;
}
/*Move* MultiTree::bestAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	ss << "State visits : " << state->n << "   MiniMax : [ ";
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r] << " ";
	}
	ss << "]" << std::endl;
	Move* move = NULL;
	QValue curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	MTNode* node;
	MTAction* a;
	for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
	{
		node = nitr->second;
		a = node->actions[role];
		
		if(!a->n)
			continue;
		if(first)
		{
			curQ = a->q;
			curA = a->id;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == a->q)
			tiebreak.push_back(a->id);
		else if(curQ < a->q)
		{
			curQ = a->q;
			curA = a->id;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadMoveInfo(role, node, a, ss);
		ss << std::endl;
	}
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	
	//dumpActionBuffer(role, ss);
	
	m_bestActionInfo = ss.str();
	return move;
}*/
Move* MultiTree::controlAction(RoleIndex role, MTState* state, QValue& qValue, bool info)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	if(!info)
	{
		ss << "State visits : " << state->n << "   MiniMax : [ ";
		for(RoleIndex r = 0 ; r < m_roleCount ; r++)
		{
			ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
		}
		ss << "]" << std::endl;
	}
	Move* move = NULL;
	QValue curQ = 0;
	QValue curGoal = 0;
	MTActionID curA = 0;
	MTActionID curgoalA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	std::vector<MTActionID> tiebreakgoal;
	MTNode* node;
	MTAction* a;
	for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
	{
		node = nitr->second;
		a = node->actions[role];
		
		if(!a->n)
			continue;
		if(first)
		{
			curQ = a->q;
			curGoal = node->getGoal(role);
			curA = a->id;
			curgoalA = a->id;
			first = false;
			tiebreak.push_back(curA);
			tiebreakgoal.push_back(curA);
		}
		else 
		{
			if(curQ == a->q)
				tiebreak.push_back(a->id);
			else if(curQ < a->q)
			{
				curQ = a->q;
				curA = a->id;
				tiebreak.clear();
				tiebreak.push_back(curA);
			}
			if(curGoal == node->getGoal(role))
				tiebreakgoal.push_back(a->id);
			else if(curGoal < node->getGoal(role))
			{
				curGoal = node->getGoal(role);
				curgoalA = a->id;
				tiebreakgoal.clear();
				tiebreakgoal.push_back(curgoalA);
			}
		}

		if(!info)
		{	
			loadMoveInfo(role, node, a, ss);
			ss << std::endl;	
		}
	}
	qValue = curQ;
	// All goals equal, default to normal handling
	if(tiebreakgoal.size() == state->nodes.size())
	{
		// Inconclusive as a control parallel info return null to indicate
		if(info)
			return NULL;
		
		if(tiebreak.size() > 1)
			move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
		else
			move = m_buffers[role][curA];
		ss << "Move selected by Default Control rule (all neighbor goals equal)\n";
	}
	else 
	{
		if(tiebreakgoal.size() > 1)
			move = m_buffers[role][tiebreakgoal[rand() % tiebreakgoal.size()]];
		else
			move = m_buffers[role][curgoalA];
		if(!info)
			ss << "Move selected by Total Control rule (highest neighbor goal)\n";
	}

		
	//dumpActionBuffer(role, ss);
	
	if(!info)
		m_bestActionInfo = ss.str();
	return move;
}
void MultiTree::loadMoveInfo(RoleIndex role, MTNode* n, MTAction* a, std::stringstream& ss)
{
	ss.width(40);
	ss << std::left << m_buffers[role][a->id]->compound.toString(m_theory->getSymbols());
	//ss.width(20);
	//ss << std::left << m_buffers[0][n->actions[0]->id]->compound.toString(m_theory->getSymbols());
	//ss.width(20);
	//ss << std::left << m_buffers[1][n->actions[1]->id]->compound.toString(m_theory->getSymbols());
	ss.width(2);
	ss << "n=";
	ss.width(8);
	ss << a->n;
	ss.width(2);
	ss << "q=";
	ss.width(10);
	ss << a->q;
	if(n)
	{
		ss.width(5);
		ss << "goal=";
		ss.width(6);
		ss << n->getGoal(role);
	}
}
Move* MultiTree::maxAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	ss << "State visits : " << state->n << "   MiniMax : [ ";
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
	}
	ss << "]";
	if(state->expanded)
		ss << " - expanded";
	else
		ss << " - not expanded";
	ss << std::endl;
	
	Move* move = NULL;
	QValue curMax = 0;
	QValue curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	std::vector<MTActionID> tiebreakTerminal;

	MTAction* a = NULL;
	MTNode* node = NULL;
	double maxVal = 0;
	for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
	{
		node = nitr->second;
		a = node->actions[role];
		maxVal = a->max > node->getGoal(0) ? a->max : node->getGoal(0);
		if(first)
		{
			curMax = maxVal;
			if(node->getTerminal())
				curQ = GOAL_MIN;
			else
				curQ = a->q;
			curA = a->id;
			first = false;
			if(node->getTerminal())
				tiebreakTerminal.push_back(curA);
			else
				tiebreak.push_back(curA);
		}
		else if(curMax == maxVal && curQ > a->q)
		{
			if(node->getTerminal())
				tiebreakTerminal.push_back(a->id);
		}
		else if(curMax == maxVal && curQ == a->q)
		{
			if(node->getTerminal())
				tiebreakTerminal.push_back(a->id);
			else
				tiebreak.push_back(a->id);
		}
		else if(curMax == maxVal && curQ < a->q)
		{
			if(node->getTerminal())
				tiebreakTerminal.push_back(a->id);
			else
			{
				curA = a->id;
				curQ = a->q;
				tiebreak.clear();
				tiebreak.push_back(a->id);
			}
		}
		else if(curMax < maxVal)
		{
			curMax = maxVal;
			curA = a->id;
			if(node->getTerminal())
				curQ = GOAL_MIN;
			else
				curQ = a->q;
			tiebreakTerminal.clear();
			tiebreak.clear();
			if(node->getTerminal())
				tiebreakTerminal.push_back(curA);
			else
				tiebreak.push_back(curA);
		}
		loadMoveMaxInfo(role, node, a, ss);
		ss << std::endl;
	}
	
	qValue = curMax;
	if(curMax == GOAL_MAX && tiebreakTerminal.size() > 0)
		move = m_buffers[role][tiebreakTerminal[0]];
	else
	{
		if(tiebreak.size() > 0)
		{
			if(tiebreak.size() > 1)
				move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
			else
				move = m_buffers[role][tiebreak[0]];
		}
		else if(tiebreakTerminal.size() == state->nodes.size())
			move = m_buffers[role][tiebreakTerminal[rand() % tiebreakTerminal.size()]];
		else if(tiebreakTerminal.size() > 0)
			move = m_buffers[role][tiebreakTerminal[0]];
		else
			std::cerr << "Warning: NULL move selected!\n";
	}
	m_bestActionInfo = ss.str();
	return move;
}
void MultiTree::loadMoveMaxInfo(RoleIndex role, MTNode* n, MTAction* a, std::stringstream& ss)
{
	ss.width(40);
	ss << std::left << m_buffers[role][a->id]->compound.toString(m_theory->getSymbols());
	ss.width(2);
	ss << "n=";
	ss.width(8);
	ss << a->n;
	ss.width(2);
	ss << "q=";
	ss.width(10);
	ss << a->q;
	ss.width(4);
	ss << "max=";
	ss.width(6);
	ss << a->max;
	ss.width(5);
	ss << "goal=";
	ss.width(6);
	ss << n->getGoal(role);
	ss.width(15);
	if(n->isTerminal())
		ss << "[terminal]";
	else if(n->isNonTerminal())
		ss << "[non-terminal]";
	else
		ss << "[unknown]";
	ss.width(15);
	if(n->expanded)
		ss << "[expanded]";
	else
		ss << "[non-expanded]";
}
Move* MultiTree::mostExploredAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	ss << "State visits : " << state->n << "   MiniMax : [ ";
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
	}
	ss << "]" << std::endl;
	Move* move = NULL;
	Visits curN = 0;
	QValue curQ = 0.0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		if(first)
		{
			curN = a->second->n;
			curQ = a->second->q;
			curA = a->first;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curN == a->second->n)
		{
			if(curQ == a->second->q)
				tiebreak.push_back(a->first);
			else if (curQ > a->second->q)
			{
				curN = a->second->n;
				curQ = a->second->q;
				curA = a->first;
				tiebreak.clear();
				tiebreak.push_back(curA);
			}
		}
		else if(curN < a->second->n)
		{
			curN = a->second->n;
			curQ = a->second->q;
			curA = a->first;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadMoveInfo(role, NULL, a->second, ss);
		ss << std::endl;
	}
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	
	//dumpActionBuffer(role, ss);
	
	m_bestActionInfo = ss.str();
	return move;
}
Move* MultiTree::goalAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	if(state->actions[role].size() == 1)
		return bestAction(role, state , qValue);
	
	bool turnTaking = true;
	for(RoleIndex r = 0 ; r < m_buffers.size() ; r++)
	{
		if(r==role)
			continue;
		if(state->actions[r].size() > 1)
		{
			turnTaking = false;
			break;
		}
	}
	
	if(!turnTaking) return bestAction(role, state , qValue);
	
	// Turntaking game where it is our turn, so factor in goals of the states the available actions lead to.
	
	std::stringstream ss;
	ss << "State visits : " << state->n << std::endl;
	Move* move = NULL;
	QValue curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	MTAction* a = NULL;
	QValue goalval = 0;
	for(MTNodeMapItr node = state->nodes.begin(); node != state->nodes.end() ; node++)
	{
		a = node->second->actions[role];
		if(!a->n)
			continue;
		goalval = node->second->child ?  a->q + goal(node->second, role) : a->q;
		if(first)
		{
			curQ = goalval;
			curA = a->id;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == goalval)
			tiebreak.push_back(a->id);
		else if(curQ < goalval)
		{
			curQ = goalval;
			curA = a->id;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadMoveGoalInfo(role, node->second, ss);
		ss << std::endl;
	}
	
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	m_bestActionInfo = ss.str();
	return move;
}
void MultiTree::loadMoveGoalInfo(RoleIndex role, MTNode* n, std::stringstream& ss)
{
	MTAction* a = n->actions[role];
	ss.width(40);
	ss << std::left << m_buffers[role][a->id]->compound.toString(m_theory->getSymbols());
	ss.width(2);
	ss << "n=";
	ss.width(8);
	ss << a->n;
	ss.width(2);
	ss << "q=";
	ss.width(10);
	ss << a->q;
	ss << "g=";
	ss.width(10);
	ss << n->getGoal(role);
	ss << "q+g=";
	ss.width(10);
	ss << a->q + n->getGoal(role);
	ss << "mm=";
	ss.width(10);
	ss << n->getMiniMaxOpti(role);
	ss << n->getMiniMaxPess(role);
}

Move* MultiTree::mmAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	//if(state->actions[role].size() == 1)
	//	return bestAction(role, state , qValue);
	
	std::stringstream ss;
	ss << "State (" << state->id << ") visits : " << state->n << "   MiniMax : [ ";
	bool solved = true;
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
		solved = solved && state->isSolved(r);
	}
	ss << "]" << (solved?" solved":"") << (state->isExpanded()?" expanded":"") << std::endl;
	Move* move = NULL;
	QValue curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	MTAction* a = NULL;
	MTAction* b = NULL;
	QValue mmpess = 0;
	QValue mmopti = 0;
	QValue mmval = 0;
	
	RoleIndex opp = role == 0 ? 1 : 0;
	
	for(MTNodeMapItr node = state->nodes.begin(); node != state->nodes.end() ; node++)
	{
		a = node->second->actions[role];
		b = node->second->actions[opp];
		//if(!a->n)
		//	continue;
		mmopti = node->second->getMiniMaxOpti(role);
		mmpess = node->second->getMiniMaxPess(role);
		if(mmopti == mmpess)
			mmval = mmpess;
		else
		{
			if(mmopti < a->q)
				mmval = mmopti;
			else 
				mmval = a->q;
		}
			
		/*mm = node->second->getMiniMaxOpti(role);
		if( mm == MM_INIT_OPTI)
			mmval = a->q;
		else
			mmval = mm + a->q/100.0;*/
		if(first)
		{
			curQ = mmval;
			curA = a->id;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == mmval)
			tiebreak.push_back(a->id);
		else if(curQ < mmval)
		{
			curQ = mmval;
			curA = a->id;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadMoveMMInfo(role, a, b, ss, mmopti, mmpess, mmval, node->second->getTerminal());
		ss << std::endl;
	}
	int unex = static_cast<int> (state->comboactions) - static_cast<int> (state->nodes.size());
	ss << "Unexplored nodes : " << unex << std::endl;
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	m_bestActionInfo = ss.str();
	return move;
}
void MultiTree::loadMoveMMInfo(RoleIndex role, MTAction* a, MTAction* b, std::stringstream& ss, QValue opti, QValue pess, QValue mmval, int t)
{
	ss.width(20);
	ss << std::left << m_buffers[role][a->id]->compound.toString(m_theory->getSymbols());
	ss.width(20);
	ss << std::left << b->move->compound.toString(m_theory->getSymbols());
	ss.width(2);
	ss << "n=";
	ss.width(8);
	ss << a->n;
	ss.width(2);
	ss << "q=";
	ss.width(10);
	ss << a->q;
	ss << "mm=";
	ss.width(3);
	ss << std::right << opti; 
	ss << "/";
	ss.width(6);
	ss << std::left << pess;
	ss << "mv=";
	ss.width(10);
	ss << mmval;
	ss << "t=";
	ss.width(10);
	ss << t;
}
Move* MultiTree::bestRAVEAction(RoleIndex role, MTState* state, QValue& qValue)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	ss << "State visits : " << state->n << "   MiniMax : [ ";
	for(RoleIndex r = 0 ; r < m_roleCount ; r++)
	{
		ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
	}
	ss << "]" << std::endl;
	Move* move = NULL;
	double curQ = 0;
	MTActionID curA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	double rave=0;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		rave = bestRAVE(a->second);
		if(first)
		{
			curQ = rave;
			curA = a->first;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == rave)
			tiebreak.push_back(a->first);
		else if(curQ < rave)
		{
			curQ = rave;
			curA = a->first;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		loadRAVEMoveInfo(role, NULL, a->second, rave, ss);
		ss << std::endl;
	}
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
	else
		move = m_buffers[role][curA];
	m_bestActionInfo = ss.str();
	return move;
}
Move* MultiTree::controlRAVEAction(RoleIndex role, MTState* state, QValue& qValue, bool info)
{
	if(state == NULL)
		return NULL;
	std::stringstream ss;
	if(!info)
	{
		ss << "State visits : " << state->n << "   MiniMax : [ ";
		for(RoleIndex r = 0 ; r < m_roleCount ; r++)
		{
			ss << state->minimax[r].opti << " " << state->minimax[r].pess << " ";
		}
		ss << "]" << std::endl;
	}
	Move* move = NULL;
	double curQ = 0;
	double curgoal = 0;
	MTActionID curA = 0;
	MTActionID curgoalA = 0;
	bool first = true;
	std::vector<MTActionID> tiebreak;
	std::vector<MTActionID> tiebreakgoal;
	double rave=0;
	MTNode* node;
	MTAction* a;
	for(MTNodeMapItr nitr = state->nodes.begin(); nitr != state->nodes.end() ; nitr++)
	{
		node = nitr->second;
		a = node->actions[role];
	
		if(!a->n)
			continue;
		rave = bestRAVE(a);
		if(first)
		{
			curQ = rave;
			curgoal = node->getGoal(role);
			curA = a->id;
			curgoalA = a->id;
			first = false;
			tiebreak.push_back(curA);
			tiebreakgoal.push_back(curA);
		}
		else 
		{
			if(curQ == rave)
				tiebreak.push_back(a->id);
			else if(curQ < rave)
			{
				curQ = rave;
				curA = a->id;
				tiebreak.clear();
				tiebreak.push_back(curA);
			}
			if(curgoal == node->getGoal(role))
				tiebreakgoal.push_back(a->id);
			else if(curgoal < node->getGoal(role))
			{
				curgoal = node->getGoal(role);
				curgoalA = a->id;
				tiebreakgoal.clear();
				tiebreakgoal.push_back(curgoalA);
			}
		}
		if(!info)
		{
			loadRAVEMoveInfo(role, node, a, rave, ss);
			ss << std::endl;
		}
	}
	qValue = curQ;
	// All goals equal, default to normal handling
	if(tiebreakgoal.size() == state->nodes.size())
	{
		// Cannot determine control move, indicate with info move as null
		if(info)
			return NULL;
		if(tiebreak.size() > 1)
			move = m_buffers[role][tiebreak[rand() % tiebreak.size()]];
		else
			move = m_buffers[role][curA];
		ss << "Move selected by Default Control rule (all neighbor goals equal)\n";
	}
	else 
	{
		if(tiebreakgoal.size() > 1)
			move = m_buffers[role][tiebreakgoal[rand() % tiebreakgoal.size()]];
		else
			move = m_buffers[role][curgoalA];
		if(!info)
			ss << "Move selected by Total Control rule (highest neighbor goal)\n";
	}
	if(!info)
		m_bestActionInfo = ss.str();
	return move;
}
double MultiTree::bestRAVE(MTAction* node)
{
	double beta = sqrt(EQUIV_PARAM/(3.0*node->state->n+EQUIV_PARAM));
	if(beta < 0.01)
		return node->q;
	return beta*node->qRAVE + (1.0-beta)*node->q;
	/*double base = 2*EQUIV_PARAM;
	if(node->state->n > base)
		return node->q;
	double alpha = node->state->n / base;
	return alpha*node->q + (1.0-alpha)*node->qRAVE;*/
}

void MultiTree::bestActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	if(state == NULL)
		return;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		
		ratings.push_back(cadiaplayer::play::MoveRating(a->second->move, m_theory->getSymbols(), a->second->n, a->second->q));
		//ratings.push_back(cadiaplayer::play::MoveRating(m_buffers[role][a->first]->toPlayString(m_theory->getSymbols()), a->second->n, a->second->q));
														
	}
}

void MultiTree::maxActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	if(state == NULL)
		return;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		
		ratings.push_back(cadiaplayer::play::MoveRating(a->second->move, m_theory->getSymbols(), a->second->n, a->second->max));
		
	}
}

void MultiTree::controlActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	if(state == NULL)
		return;
	
	QValue score = 0.0;
	Move* controlMove = controlAction(role, state, score, true);
	if(controlMove)
		ratings.push_back(cadiaplayer::play::MoveRating(controlMove, m_theory->getSymbols(), 1, score));
	else
		bestActionRatings(role, state, ratings);
}
void MultiTree::bestRAVEActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	if(state == NULL)
		return;
	for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end() ; a++)
	{
		if(!a->second->n)
			continue;
		
		ratings.push_back(cadiaplayer::play::MoveRating(a->second->move, m_theory->getSymbols(), a->second->n, bestRAVE(a->second)));
	}
}

void MultiTree::controlRAVEActionRatings (RoleIndex role, MTState* state, cadiaplayer::play::MoveRatings& ratings)
{
	if(state == NULL)
		return;
	
	QValue score = 0.0;
	Move* controlMove = controlRAVEAction(role, state, score, true);
	if(controlMove)
		ratings.push_back(cadiaplayer::play::MoveRating(controlMove, m_theory->getSymbols(), 1, score));
	else
		bestRAVEActionRatings(role, state, ratings);
}
						  
						  
void MultiTree::loadRAVEMoveInfo(RoleIndex role, MTNode* n, MTAction* a, QValue rave, std::stringstream& ss)
{
	ss.width(40);
	ss << std::left << m_buffers[role][a->id]->compound.toString(m_theory->getSymbols());
	//ss.width(20);
	//ss << std::left << m_buffers[0][n->actions[0]->id]->compound.toString(m_theory->getSymbols());
	//ss.width(20);
	//ss << std::left << m_buffers[1][n->actions[1]->id]->compound.toString(m_theory->getSymbols());
	ss.width(2);
	ss << "n=";
	ss.width(8);
	ss << a->n;
	ss.width(2);
	ss << "q=";
	ss.width(10);
	ss << a->q;
	if(n)
	{
		ss.width(5);
		ss << "goal=";
		ss.width(6);
		ss << n->getGoal(role);
	}
	
	ss.width(6);
	ss << "nRAVE=";
	ss.width(8);
	ss << a->nRAVE;
	
	ss.width(6);
	ss << "qRAVE=";
	ss.width(10);
	ss << a->qRAVE;
	
	ss.width(5);
	ss << "RAVE=";
	ss.width(10);
	ss << rave;
}
void MultiTree::dropDepth(Depth depth)
{
	if(m_tree.size() <= depth || m_tree[depth] == NULL)
		return;
	for(MTStateMapItr itr = m_tree[depth]->begin() ; itr != m_tree[depth]->end() ; itr++)
	{
		if(itr->second->isSolved(0))
			continue;
		itr->second->rmRef();
		//if(depth > 7)
		//	std::cerr << "Ref count of " << itr->second->id << " dec to " << itr->second->refs << std::endl;
		if(itr->second->hasRef())
			continue;
		for(MTActionsItr map = itr->second->actions.begin() ; map != itr->second->actions.end() ; map++)
		{
			for(MTActionMapItr action = map->begin() ; action != map->end() ; action++)
			{
				MTAction* a = action->second;
				m_actionRecycler.push(a);
				m_actionsDropped++;
			}
			map->clear();
		}
		for(MTNodeMapItr node = itr->second->nodes.begin() ; node != itr->second->nodes.end() ; node++)
		{
			node->second->reset();
			m_nodeRecycler.push(node->second);
			m_nodesDropped++;
		}
		itr->second->nodes.clear();
		//if(depth > 7)
		//	std::cerr << "State dropped at depth " << depth <<  " with id:" << itr->second->id << std::endl;
		m_stateRecycler.push(itr->second);
		m_transpositions.erase(itr->second->id);
		m_statesDropped++;
		//delete itr->second;
	}
	m_tree[depth]->clear();
	delete m_tree[depth];
	m_tree[depth] = NULL;
}
void MultiTree::drop()
{
	for(Depth d = 0 ; d < m_tree.size() ; d++)
	{
		dropDepth(d);
	}
}
void MultiTree::destroyDepth(Depth depth)
{
	if(m_tree.size() <= depth || m_tree[depth] == NULL)
		return;
	for(MTStateMapItr itr = m_tree[depth]->begin() ; itr != m_tree[depth]->end() ; itr++)
	{
		itr->second->rmRef();
		if(itr->second->hasRef())
			continue;
		for(MTActionsItr map = itr->second->actions.begin() ; map != itr->second->actions.end() ; map++)
		{
			for(MTActionMapItr action = map->begin() ; action != map->end() ; action++)
			{
				delete action->second;
			}
		}
		for(MTNodeMapItr node = itr->second->nodes.begin() ; node != itr->second->nodes.end() ; node++)
		{
			delete node->second;
		}
		m_transpositions.erase(itr->second->id);
		delete itr->second;
	}
	m_tree[depth]->clear();
	delete m_tree[depth];
	m_tree[depth] = NULL;
}
void MultiTree::destroy()
{
	for(Depth d = 0 ; d < m_tree.size() ; d++)
	{
		destroyDepth(d);
	}
}
void MultiTree::reset()
{
	this->destroy();
}

MTNode* MultiTree::makeAction(MTStateID sid, Depth depth, PlayMoves& moves)
{
#ifdef DEBUG_MULTITREE
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		std::cerr << "[" << moves[n]->compound.toString(m_theory->getSymbols()) << "{" << depth << "}]\n";
	}
#endif
	MTNode* node = NULL;
	MTState* state = lookupState(sid, depth);
	if(state != NULL)
	{
		node = createNode(state, moves);
		if(m_muteretract && state->prepared)
		{ 
			if(node->child != NULL && node->child->prepared)
			{
					
				if(m_theory->gotoState(node->child->facts, true))
				{
#ifdef DEBUG_MULTITREE
					std::cerr << "Move to state: " << node->child->id << "\n" << m_theory->stateFactsToString(node->child->facts);
#endif
					//if(m_currentDepth == depth && !node->hasGoal(0))
					if(!node->hasGoal(0))
					{
						//std::cerr << "Setting goal for ready node " << node->actions[0]->move->compound.toString(m_theory->getSymbols()) << " and " << node->actions[1]->move->compound.toString(m_theory->getSymbols()) << std::endl;
						for(size_t n = 0 ; n < m_theory->getRoles()->size() ; n++)
						{
							goal(node, n);
						}
					}
					return node;
				}
			}
			m_theory->syncController(state->facts);
		}
	}
	m_theory->make(moves);
	if(node != NULL)
	{
		if(node->isUnknown())
		{
			if(m_theory->isTerminal())
				node->makeTerminal();
			else
				node->makeNonTerminal();
		}
		//if(m_currentDepth == depth && !node->hasGoal(0))
		if(!node->hasGoal(0))	
		{
			//std::cerr << "Setting goal for " << node->actions[0]->move->compound.toString(m_theory->getSymbols()) << " and " << node->actions[1]->move->compound.toString(m_theory->getSymbols()) << std::endl;
			for(size_t n = 0 ; n < m_theory->getRoles()->size() ; n++)
			{
				node->setGoal(n, m_theory->goal(n));
			}
		}
	}
#ifdef DEBUG_MULTITREE
	std::cerr << "Move to state: (by theory)\n" << GameTheory::listToString(m_theory->getStateRef(), m_theory->getSymbols());
#endif
	return node;
}

void MultiTree::retractAction()
{
	m_theory->retract();
}
void MultiTree::muteRetract()
{
	m_theory->muteRetract();
	m_muteretract = true;
}
void MultiTree::syncRetract()
{
	m_theory->syncRetract();
	m_muteretract = false;
}


bool MultiTree::getMoves(RoleIndex role, MTStateID id, Depth depth, RoleMoves& moves)
{
#ifdef DEBUG_MULTITREE
	std::cerr << "processing getMoves\n";
#endif
	MTState* state = lookupState(id, depth);
	if(state != NULL && state->prepared > role && !state->actions[role].empty())
	{
		for(MTActionMapItr a = state->actions[role].begin(); a != state->actions[role].end();++a)
		{
			moves.push_back(m_buffers[role][a->first]);		
		}
#ifdef DEBUG_MULTITREE
		std::cerr << "returning " << moves.size() << " moves\n";
#endif
		return moves.size();
	}
#ifdef DEBUG_MULTITREE
	std::cerr << "state not ready to provide actions\n";
#endif
	RoleMoves* tm = m_theory->getMoves(role);
	for(size_t n = 0 ; n < tm->size() ; ++n)
	{
		moves.push_back((*(tm))[n]);
	}
#ifdef DEBUG_MULTITREE
	std::cerr << "returning " << moves.size() << " moves from theory\n";
#endif
	return moves.size(); 
}
bool MultiTree::isTerminal()
{
	MTNode* node = peekTrail();
	if(node == NULL)
		return m_theory->isTerminal();
	
	if(node->isTerminal())
	{
	//	std::cerr << "#" << node->parent->id << "/" << node->actions[0]->n << "/" << node->actions[0]->q << "#";
		return true;
	}
	if(node->isNonTerminal())
		return false;
	
	//std::cerr << "{Unknown terminal}";
	if(m_theory->isTerminal())
		node->makeTerminal();
	else
		node->makeNonTerminal();
	return node->isTerminal();
}
QValue MultiTree::goal(RoleIndex role)
{
	MTNode* node = peekTrail();
	if(node == NULL)
		return m_theory->goal(role);
	
	if(node->hasGoal(role))
		return node->getGoal(role);
	
	if(node->child != NULL)
		m_theory->syncController(node->child->facts);
	
	//std::cerr << "{Unknown goal}";
	QValue g = m_theory->goal(role);
	node->setGoal(role, g);
	return g;
}
QValue MultiTree::goal(MTNode* node, RoleIndex role)
{
	if(node->hasGoal(role))
		return node->getGoal(role);
	
	if(node->child == NULL)
		return cadiaplayer::play::GOAL_MIN;
		
	if(node->child != NULL)
		m_theory->gotoState(node->child->facts);
	
	//std::cerr << "{Unknown goal}";
	QValue g = m_theory->goal(role);
	node->setGoal(role, g);
	m_theory->retract();
	return g;
}
Move* MultiTree::getBufferedMove(RoleIndex role, Move* move)
{
	MTActionID id = m_theory->getGameStateID(move);
	MTActionBufferItr itr = m_buffers[role].find(id);
	if(itr == m_buffers[role].end())
	{
		Move* temp = new Move(*move);
		m_buffers[role][id] = temp;
		return temp;
	}
	return itr->second;
}
Move* MultiTree::getBufferedMove(RoleIndex role, MTActionID id)
{
	MTActionBufferItr itr = m_buffers[role].find(id);
	if(itr == m_buffers[role].end())
		return NULL;
	return itr->second;
}
MTNode* MultiTree::pushTrail(MTStateID id, Depth depth, PlayMoves& moves)
{
	MTState* s = lookupState(id, depth);
	if(s == NULL)
		return pushTrail(NULL);
	MTNode* node = createNode(s, moves);
	return pushTrail(node);
}
MTNode* MultiTree::pushTrail(MTNode* node)
{
	if(m_trail->size() > 0 && node != NULL && m_trail->back() != NULL)
		m_trail->back()->child = node->parent;
	m_trail->push_back(node);
	return node;
}
MTNode*	MultiTree::popTrail()
{
	if(m_trail->empty())
		return NULL;
	MTNode* node = m_trail->back();
	m_trail->pop_back();
	return node;
}
MTNode* MultiTree::peekTrail()
{
	if(m_trail->empty())
		return NULL;
	return m_trail->back();
}
MTNode* MultiTree::peekTrailParent()
{
	if(m_trail->size() < 2)
		return NULL;
	MTNode* temp = m_trail->back();
	m_trail->pop_back();
	MTNode* parent = m_trail->back();
	m_trail->push_back(temp);
	return parent;
}
void MultiTree::terminateTrail(MTStateID id, Depth depth)
{
	if(peekTrail()!=NULL)
	{
		MTState* s = lookupState(id, depth);
		peekTrail()->child = s;
		peekTrail()->makeTerminal();
	}
}
void MultiTree::resetTrail()
{
	m_nodeAdded = NULL;
	m_trail->clear();
}
MTTrail* MultiTree::hijackTrail()
{
	MTTrail* temp = m_trail;
	m_trail = new MTTrail();
	return temp;
}				

MTState* MultiTree::lookupState(MTStateID id, Depth depth)
{
/*	if(m_tree.size() <= depth)
	{
#ifdef DEBUG_MULTITREE
		std::cerr << "State out of tree scope\n";
#endif
		return NULL;
	}*/
	while(depth >= m_tree.size())
	{
		MTStateMap* map(new MTStateMap());
		m_tree.push_back(map);
	}
	if(!m_tree[depth])
		return NULL;
	MTStateMapItr itr = m_tree[depth]->find(id);
	if(itr == m_tree[depth]->end())
	{
		itr = m_transpositions.find(id);
		if(itr == m_transpositions.end())
		{
#ifdef DEBUG_MULTITREE
			std::cerr << "State not found\n";
#endif
			return NULL;
		}
		else
		{
			(*(m_tree[depth]))[id]=itr->second;
			itr->second->addRef();
			//if(depth > 7)
			//	std::cerr << "LU:Ref of " << itr->second->id << " inc to " << itr->second->refs << " at depth " << depth << std::endl;

		}
	}
	return itr->second;
}
MTState* MultiTree::lookupState(GameState* s, Depth depth)
{
	return lookupState(m_theory->getGameStateID(s), depth);
}
Move* MultiTree::lookupMove(RoleIndex role, MTActionID id)
{
	MTActionBufferItr itr = m_buffers[role].find(id);
	if(itr == m_buffers[role].end())
		return NULL;
	return itr->second;
}
MTAction* MultiTree::lookupAction(RoleIndex role, MTState* state, MTActionID id)
{
	if(state == NULL)
		return NULL;
	MTActionMap& map = state->actions[role];
	MTActionMapItr itr = map.find(id);
	if(itr == map.end())
		return NULL;
	return itr->second;
}
MTAction* MultiTree::lookupAction(RoleIndex role, MTState* state, Move* move)
{
	return lookupAction(role, state, m_theory->getGameStateID(move));
}
MTAction* MultiTree::lookupAction(RoleIndex role, GameState* state, Move* move, Depth depth)
{
	MTStateID sid = m_theory->getGameStateID(state);
	MTState* s = lookupState(sid, depth);
	if( s == NULL)
		return NULL;
	return lookupAction(role, s, move);
}
MTNode* MultiTree::lookupNode(MTState* state, PlayMoves& moves)
{
	MTActionID combo = MTState::squashIDs(m_theory, moves);	
	MTNodeMapItr itr = state->nodes.find(combo);
	if(itr == state->nodes.end())
		return NULL;
	return itr->second;
}
MTNode* MultiTree::lookupSingleNode(MTState* state, Move* move)
{
	MTActionID id = m_theory->getGameStateID(move);	
	MTNodeMapItr itr = state->nodes.find(id);
	if(itr == state->nodes.end())
		return NULL;
	return itr->second;
}

MTState* MultiTree::createState(MTStateID id, Depth depth, GameState* state)
{
	while(depth >= m_tree.size())
	{
		MTStateMap* map(new MTStateMap());
		m_tree.push_back(map);
	}
	MTStateMapItr itr = m_tree[depth]->find(id);
	if(itr == m_tree[depth]->end())
	{
		MTState* sp;
		itr = m_transpositions.find(id);
		if(itr == m_transpositions.end())
		{
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
			m_transpositions[id] = sp;
//#ifdef DEBUG_MULTITREE
			//if(depth > 7)
			//	std::cerr << "State created d:" << depth << " id:" << id << "\n" ;//<< GameTheory::listToString(state, m_theory->getSymbols());
//#endif			
		}
		else
		{
			sp = itr->second;
		}
		(*(m_tree[depth]))[id] = sp;
		sp->addRef();
		//if(depth > 7)
		//	std::cerr << "CR:Ref of " << sp->id << " inc to " << sp->refs << std::endl;
		return sp;	
	}
	return itr->second;
}
Move* MultiTree::createMove(RoleIndex role, Move* move, MTActionID id)
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
MTAction* MultiTree::createAction(RoleIndex role, MTState* state, Move* move)
{
	MTActionID id = m_theory->getGameStateID(move);
	return createAction(role, state, move, id);
}
MTAction* MultiTree::createAction(RoleIndex role, MTState* state, Move* move, MTActionID hint)
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
MTNode* MultiTree::createNode(MTState* state, PlayMoves& moves, MTState* child)
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
#ifdef DEBUG_MULTITREE
		std::cerr << "Node created with actions";
#endif
		MTAction* a;
		for(size_t n = 0 ; n < moves.size() ; n++)
		{
			a = createAction(n, state, moves[n]);
			np->actions.push_back(a);
			if(state->comboactions == state->actions[n].size())
				a->node = np;
#ifdef DEBUG_MULTITREE
			std::cerr << " " << moves[n]->compound.toString(m_theory->getSymbols());
#endif
		}
#ifdef DEBUG_MULTITREE
		std::cerr << std::endl;
#endif
		np->goals.resize(moves.size(), cadiaplayer::play::GOAL_UNKNOWN);
#ifdef DEBUG_MULTITREE
std::cerr << "Node created\n";
#endif
		return np;
	}
	return itr->second;
}

MTNode* MultiTree::createStateAndNode(RoleIndex role, GameState* state, Move* move, Depth depth, PlayMoves& moves, GameStateID child)
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
	return createNode(s, moves, lookupState(child, depth+1));
}

MTState* MultiTree::setupState(GameState* state, Depth depth, AllRoleMoves& moves)
{
	MTStateID sid = m_theory->getGameStateID(state); 
	MTState* s = createState(sid, depth, state);
	if(s->prepared == m_roleCount)
		return s;
	s->prepared = m_roleCount;
	MTActionID aid = 0; 
	s->comboactions = 1;
	for(RoleIndex role = 0 ; role < m_roleCount ; role++)
	{
		for(size_t move = 0 ; move < moves[role].size() ; ++move)
		{
			aid = m_theory->getGameStateID(moves[role][move]);
			Move* m = createMove(role, moves[role][move], aid);
			createAction(role, s, m, aid); 
		}
		s->comboactions *= s->actions[role].size();
	}
	return s;
}
MTState* MultiTree::setupState(GameState* state, Depth depth, RoleIndex role, RoleMoves& moves)
{
	MTStateID sid = m_theory->getGameStateID(state); 
	MTState* s = createState(sid, depth, state);
	if(s->prepared == m_roleCount)
		return s;
	s->prepared++;
	MTActionID aid = 0; 
	if(s->prepared == 1)
		s->comboactions = 1;
	for(size_t move = 0 ; move < moves.size() ; ++move)
	{
		aid = m_theory->getGameStateID(moves[move]);
		Move* m = createMove(role, moves[move], aid);
		createAction(role, s, m, aid); 
	}
	s->comboactions *= s->actions[role].size();
	return s;
}
bool MultiTree::verifyTransition(MTState* state, MTPlayMoves& moves, PlayMoves& pm)
{
/*
	std::cerr << "v: ";
	for(int n = 0 ; n < moves.size() ; ++n)
	{
		if(!moves[n])
		{
			std::cerr << "(null) ";
			continue;
		}
		Move* move = m_buffers[n][moves[n]];
		std::cerr << move->toString(m_theory->getSymbols()) << " ";
	} 
	std::cerr << std::endl;
*/
	MTActionMapItr itr;
	for(Visits n = 0 ; n < moves.size() ; n++)
	{
		itr = state->actions[n].find(moves[n]); 
		if(itr == state->actions[n].end())
			return false;
		pm.push_back(itr->second->move);
	}	
	return true;
}

Move* MultiTree::verifyMove(MTState* state, GameStateID move, cadiaplayer::play::RoleIndex role)
{
	MTActionMapItr itr = state->actions[role].find(move); 
	if(itr == state->actions[role].end())
		return NULL;
	return itr->second->move;
}

std::string	MultiTree::pathToString(cadiaplayer::play::utils::MoveSelectionData& path)
{
	std::stringstream ss;
	for(int n = 0 ; n < path.getEnd() ; ++n)
	{
		ss << n << ") ";
		for(int role = 0 ; role < m_roleCount ; ++role)
		{
			GameStateID id = path.getMoveId(role, n);
			ss << "[" << id << "] ";
			Move* move = getBufferedMove(role, id);
			if(move)
				ss << move->toString(m_theory->getSymbols()) << " ";
			else
				ss << "NULL ";
		}
		ss << std::endl;
	} 
	return ss.str();
}

std::string MultiTree::getMemoryInfo(void)
{
	if(m_memoryInfo != "")
		return m_memoryInfo;
	std::stringstream ss;
	ss << "States Dropped     : " << m_statesDropped			<< std::endl;
	ss << "States Reused      : " << m_statesReused				<< std::endl;
	ss << "States Created     : " << m_statesCreated			<< std::endl;
	ss << "States Available   : " << m_stateRecycler.size()		<< std::endl;
	ss << std::endl;
	ss << "Actions Dropped    : " << m_actionsDropped			<< std::endl;
	ss << "Actions Reused     : " << m_actionsReused			<< std::endl;
	ss << "Actions Created    : " << m_actionsCreated			<< std::endl;
	ss << "Actions Available  : " << m_actionRecycler.size()	<< std::endl;
	ss << std::endl;
	ss << "Nodes Dropped      : " << m_nodesDropped				<< std::endl;
	ss << "Nodes Reused       : " << m_nodesReused				<< std::endl;
	ss << "Nodes Created      : " << m_nodesCreated				<< std::endl;
	ss << "Nodes Available    : " << m_nodeRecycler.size()		<< std::endl;
	ss << std::endl;
	for(RoleIndex role = 0 ; role < m_roleCount ; role++)
	{
		ss << "Move buffer " << role << " size : " << m_buffers[role].size() << std::endl;
	}
	m_memoryInfo = ss.str();
	m_memoryInfo = ss.str();
	return m_memoryInfo;
}
void MultiTree::resetMemoryInfo(void)
{
	m_statesCreated		= 0;
	m_statesReused		= 0;
	m_statesDropped		= 0;
	m_actionsCreated	= 0;
	m_actionsReused		= 0;
	m_actionsDropped	= 0;
	m_nodesCreated		= 0;
	m_nodesReused		= 0;
	m_nodesDropped		= 0;
	m_memoryInfo		= "";
}

/* File Formatting:
[Number of roles]
[Number of Depths]
  [Depth number] [Size of Depth]
    [ID of State] [State visits] [Number of nodes in State]
      [Child State of Node]
	[Node role Action Info] # xNumber of Roles
*/
void MultiTree::dumpToFile(std::string filename, GameState* s, Depth d)
{
	std::queue<MTState*>* current = new std::queue<MTState*>();
	std::queue<MTState*>* next = new std::queue<MTState*>();
	std::queue<MTState*>* swap = NULL;
	
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc);
	file << m_roleCount << std::endl;
	file << m_tree.size()-d << std::endl;
	
	file << "0" << std::endl;
	file << "1" << std::endl;	
	MTState* state = lookupState(s, d);
	dumpState(state, file, next);

	while(!next->empty())
	{
		swap = next;
		next = current;
		current = swap;
		d++;

		file << d << std::endl;
		file << current->size() << std::endl;	
		
		while(!current->empty())
		{
			state = current->front();
			current->pop();
			dumpState(state, file, next);
		}
	}

	file.close();

	delete current;
	delete next;
}

void MultiTree::dumpState(MTState* state, std::ofstream& file, std::queue<MTState*>* children)
{	
	file << state->id << std::endl;
	file << state->n << std::endl;
	for(size_t m = 0 ; m < state->minimax.size() ; m++)
	{
		file << state->minimax[m].opti << " " << state->minimax[m].pess << " ";
	}
	file << std::endl;
	file << state->nodes.size() << std::endl;
	for(MTNodeMapItr nodeEntry = state->nodes.begin() ; nodeEntry != state->nodes.end() ; nodeEntry++)
	{
		MTNode* node = nodeEntry->second;
		if(node->child == NULL)
			file << "0" << std::endl;
		else
		{
			file << node->child->id << std::endl;
			children->push(node->child);
		}
		for(size_t a = 0 ; a < node->actions.size() ; a++)
		{
			file << node->actions[a]->move->compound.toString(m_theory->getSymbols()) << "  ";
			file << "n=" << node->actions[a]->n << "  ";
			file << "q=" << node->actions[a]->q << "  ";
			if(m_roleCount == 1)
			   file << "max=" << node->actions[a]->max;
			file << std::endl;
		}
	}	
}

void MultiTree::dumpActionBuffer(RoleIndex role, std::stringstream& ss)
{
	MTActionBuffer& buffer = m_buffers[role];
	for(MTActionBufferItr itr = buffer.begin() ; itr != buffer.end() ; itr++)
	{
		if(itr->second)
			ss << itr->second->compound.toString(m_theory->getSymbols()) << "\t\t:\t" << itr->second->getQ() << std::endl;
		else
			ss << "NULL move detected\n";
	}
}

void MultiTree::writeTreeToFile(std::string filename, GameState* s)
{
	MTStateMap transp;
	MTState* state = lookupState(s, 0);
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc);
	file << m_roleCount << std::endl;
	
	traverseTreeForWriting(transp, state, file);
	
	file.close(); 
}

void MultiTree::traverseTreeForWriting(MTStateMap& transp, MTState* state, std::ofstream& file)
{
	std::queue<MTState*> children;
	
	transp[state->id] = NULL;
	
	writeStateToFile(state, file, &children);
	
	while(!children.empty())
	{
		MTState* child = children.front();
		children.pop();
		
		MTStateMapItr itr = transp.find(child->id);
		if(itr != transp.end())
			continue;
		traverseTreeForWriting(transp, child, file); 
	}
}

void MultiTree::writeStateToFile(MTState* state, std::ofstream& file, std::queue<MTState*>* children)
{	
	file << state->id << std::endl;
	file << state->n << std::endl;
	file << state->nodes.size() << std::endl;
	for(MTNodeMapItr nodeEntry = state->nodes.begin() ; nodeEntry != state->nodes.end() ; nodeEntry++)
	{
		MTNode* node = nodeEntry->second;
		if(node->child == NULL)
			file << "0" << std::endl;
		else
		{
			file << node->child->id << std::endl;
			children->push(node->child);
		}
		for(size_t a = 0 ; a < node->actions.size() ; a++)
		{
			file << node->actions[a]->move->compound.toString(m_theory->getSymbols()) << "  ";
			file << "n=" << node->actions[a]->n << "  ";
			file << "q=" << node->actions[a]->q << "  ";
			if(m_roleCount == 1)
				file << "max=" << node->actions[a]->max;
			file << std::endl;
		}
	}	
}
