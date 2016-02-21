#include "gamestatespace.h"
using namespace cadiaplayer::play::utils;

GameStateSpace::GameStateSpace():
m_theory(NULL),
m_creationCounter(0)
{
	// If this constructor is used a theory must be set using setTheory before 
	// using this class.
}
GameStateSpace::GameStateSpace(GameTheory* t, RoleIndex ri):
m_creationCounter(0)
{
	setTheory(t, ri);
}
void GameStateSpace::setTheory(GameTheory* t, RoleIndex ri)
{
	m_theory = t;
	m_roleindex = ri;
	// Build the Zobrist key mappings
	//buildKeymap();
	// Initialize the statespace
	buildStateSpace();
	m_moveInitQ = INIT_AVG;
}
bool GameStateSpace::isTheorySet(void)
{
	return m_theory != NULL;
}
GameStateSpace::~GameStateSpace(void)
{
	drop();
	ActionBuffer::iterator del;
	for(del = m_actionbuffer.begin() ; del != m_actionbuffer.end() ; ++del)
	{
		delete (*del).second;
	}
	m_actionbuffer.clear();
}

ZKEY GameStateSpace::getZKey(cadiaplayer::play::parsing::Compound* c)
{
	return m_theory->getGameStateID(c);
}
ZKEY GameStateSpace::getZKey(Move* m)
{
	return getZKey(&(m->compound));
}
ZKEY GameStateSpace::getZKey(GameState* s)
{
	return m_theory->getGameStateID(s);
}
void GameStateSpace::prepareState(GameState* s, RoleMoves& moves, unsigned int depth)
{
	ZKEY statekey = getZKey(s);
	State* state = lookupAndCreateState(depth, statekey, s); 
	state->prepared = true;
	for(size_t n = 0 ; n < moves.size() ; ++n)
	{
		lookupAndCreateAction(state, moves[n], getZKey(moves[n])); 
	}
}
QNode* GameStateSpace::updateValue(QNode* node, double reward)
{
	// Update the Q node incrementally :
	// NewEstimate <- OldEstimate + StepSize[Target - OldEstimate]
	
	if(node == NULL)
		return NULL;
	
	// Note down this visit
	node->state->visits++;
	node->n++;
	node->q = node->q + (1.0/node->n)*(reward - node->q);
	if(node->max < reward)
		node->max=reward;
	return node;
}
QNode* GameStateSpace::updateValue(GameState* s, Move* move, unsigned int depth, double reward)
{
	ZKEY statekey = getZKey(s);
	State* state = lookupAndCreateState(depth, statekey, s);

#ifdef LOG_UCT_TREE
	m_treelog.bufferTrail(statekey);
#endif	
	
	// Has this move been seen yet for this gamestate?
	ZKEY actionkey = getZKey(move);
	if(actionkey == 0)
		return NULL;
	QNode* a = lookupAndCreateAction(state, move, actionkey);
	return updateValue(a, reward);
}
cadiaplayer::play::Move* GameStateSpace::bestAction(GameState* s, unsigned int depth, double& qValue)
{
	std::stringstream ss;
	ZKEY statekey = getZKey(s);
	State* state = lookupState(depth, statekey);
	if(state == NULL)
		return NULL;
	Move* move = NULL;
	double curQ = 0;
	ZKEY curA = 0;
	bool first = true;
	std::vector<ZKEY> tiebreak;
	for(ActionMap::iterator a = state->actions.begin(); a != state->actions.end() ; a++)
	{
		if(first)
		{
			curQ = (*a).second->q;
			curA = (*a).first;
			first = false;
			tiebreak.push_back(curA);
		}
		else if(curQ == (*a).second->q)
			tiebreak.push_back((*a).first);
		else if(curQ < (*a).second->q)
		{
			curQ = (*a).second->q;
			curA = (*a).first;
			tiebreak.clear();
			tiebreak.push_back(curA);
		}
		ss << m_actionbuffer[(*a).first]->compound.toString(m_theory->getSymbols()) << "\tn=" << (*a).second->n << "\tq=" << (*a).second->q << std::endl;
	}
	qValue = curQ;
	if(tiebreak.size() > 1)
		move = m_actionbuffer[tiebreak[rand() % tiebreak.size()]];
	else
		move = m_actionbuffer[curA];
	m_bestActionInfo = ss.str();
	return move;
}

cadiaplayer::play::Move* GameStateSpace::maxAction(GameState* s, unsigned int depth, double& qValue)
{
	std::stringstream ss;
	ZKEY statekey = getZKey(s);
	State* state = lookupState(depth, statekey);
	if(state == NULL)
		return NULL;
	Move* move = NULL;
	double curMax = 0;
	double curQ = 0;
	ZKEY curA = 0;
	bool first = true;
	std::vector<ZKEY> tiebreak;
	std::vector<ZKEY> tiebreakTerminal;
	//int count = 0;
	//std::cerr << "ActionMap of State is of size : " << state->actions.size() << std::endl; 
	for(ActionMap::iterator a = state->actions.begin(); a != state->actions.end() ; a++)
	{
		//count++;
		if(first)
		{
			curMax = (*a).second->max;
			if((*a).second->toTerminal)
				curQ = GOAL_MIN;
			else
				curQ = (*a).second->q;
			curA = (*a).first;
			first = false;
			if((*a).second->toTerminal)
				tiebreakTerminal.push_back(curA);
			else
				tiebreak.push_back(curA);
		}
		else if(curMax == (*a).second->max && curQ > (*a).second->q)
		{
			if((*a).second->toTerminal)
				tiebreakTerminal.push_back((*a).first);
		}
		else if(curMax == (*a).second->max && curQ == (*a).second->q)
		{
			if((*a).second->toTerminal)
				tiebreakTerminal.push_back((*a).first);
			else
				tiebreak.push_back((*a).first);
		}
		else if(curMax == (*a).second->max && curQ < (*a).second->q)
		{
			if((*a).second->toTerminal)
				tiebreakTerminal.push_back((*a).first);
			else
			{
				curA = (*a).first;
				curQ = (*a).second->q;
				tiebreak.clear();
				tiebreak.push_back((*a).first);
			}
		}
		else if(curMax < (*a).second->max)
		{
			curMax = (*a).second->max;
			curA = (*a).first;
			if((*a).second->toTerminal)
				curQ = GOAL_MIN;
			else
				curQ = (*a).second->q;
			tiebreakTerminal.clear();
			tiebreak.clear();
			if((*a).second->toTerminal)
				tiebreakTerminal.push_back(curA);
			else
				tiebreak.push_back(curA);
		}
		if((*a).second->toTerminal)
			ss << m_actionbuffer[(*a).first]->compound.toString(m_theory->getSymbols()) 
			<< "\tn=" << (*a).second->n << "\tq=" << (*a).second->q << "\tmax=" << (*a).second->max << "\t[terminal]" << std::endl;
		else
			ss << m_actionbuffer[(*a).first]->compound.toString(m_theory->getSymbols()) 
			<< "\tn=" << (*a).second->n << "\tq=" << (*a).second->q << "\tmax=" << (*a).second->max << "\t[non-terminal]" << std::endl;
	}
	qValue = curMax;
	if(curMax == GOAL_MAX && tiebreakTerminal.size() > 0)
		move = m_actionbuffer[tiebreakTerminal[0]];
	else
	{
		if(tiebreak.size() > 0)
		{
			if(tiebreak.size() > 1)
				move = m_actionbuffer[tiebreak[rand() % tiebreak.size()]];
			else
				move = m_actionbuffer[tiebreak[0]];
		}
		else
			move = m_actionbuffer[tiebreakTerminal[0]];
	}
	m_bestActionInfo = ss.str();
	return move;
}
double GameStateSpace::maxQValue(GameState* s, unsigned int depth)
{
	ZKEY statekey = getZKey(s);
	State* state = lookupState(depth, statekey);
	if(state == NULL)
		return 0;
	return maxQValue(state);
}
double GameStateSpace::maxQValue(QNode* s)
{
	return maxQValue(s->state);
}
double GameStateSpace::maxQValue(State* s)
{
	double max = 0;
	for(ActionMap::iterator a = s->actions.begin(); a != s->actions.end() ; a++)
	{
		if(a->second == NULL || a->second->n == 0)
			continue;
		if(a->second->q > max)
			max = a->second->q;
	} 
	return max;
}

QNode* GameStateSpace::getNode(GameState* s, Move* move, unsigned int depth)
{
	ZKEY statekey = getZKey(s);
	State* state = lookupState(depth, statekey);
	if(state == NULL)
		return NULL;
	ZKEY actionkey = getZKey(move);
	return lookupAction(state, actionkey);
}


void GameStateSpace::dropDepth(unsigned int depth)
{
	if(depth >= m_statespace.size())
		return;
	for(StateMap::iterator sit = m_statespace[depth]->begin() ; sit != m_statespace[depth]->end() ; sit++)
	{
		for(ActionMap::iterator ait = (*sit).second->actions.begin() ; ait != (*sit).second->actions.end() ; ait++)
		{
			delete (*ait).second;	
		}
		(*sit).second->actions.clear();
#ifdef LOG_UCT_TREE
		m_treelog.deleteNode(sit->first, depth);
#endif
		delete (*sit).second;
	}
	m_statespace[depth]->clear();
	// done in destructor
	//delete statespace[depth];
}

void GameStateSpace::drop()
{
#ifdef LOG_UCT_TREE
	size_t depth = 0;
#endif
	for(StateSpace::iterator dit = m_statespace.begin() ; dit != m_statespace.end() ; dit++)
	{
		for(StateMap::iterator sit = (*dit)->begin() ; sit != (*dit)->end() ; sit++)
		{
			for(ActionMap::iterator ait = (*sit).second->actions.begin() ; ait != (*sit).second->actions.end() ; ait++)
			{
				delete (*ait).second;	
			}
			(*sit).second->actions.clear();
#ifdef LOG_UCT_TREE
			m_treelog.deleteNode(sit->first, depth);
#endif
			delete (*sit).second;
		}
#ifdef LOG_UCT_TREE
		depth++;
#endif
		(*dit)->clear();
		delete (*dit);
	}
	m_statespace.clear();
}
void GameStateSpace::reset()
{
	drop();
	buildStateSpace();
}

void GameStateSpace::getMoves(GameTheory& theory, RoleIndex role, unsigned int depth, RoleMoves& moves)
{
	State* state = lookupState(depth, getZKey(theory.getStateRef()));
	if(state != NULL)
	{
		if(!state->prepared)
		{
			RoleMoves* temp = theory.getMoves(role);
			prepareState(theory.getStateRef(), *temp, depth);
		}
		for(ActionMap::iterator a = state->actions.begin(); a != state->actions.end();++a)
		{
			moves.push_back(m_actionbuffer[a->first]);		
		}
		return;
	}
	RoleMoves* tm = theory.getMoves(role);
	for(size_t n = 0 ; n < tm->size() ; ++n)
	{
		moves.push_back((*(tm))[n]);
	}
	return; 
}
cadiaplayer::play::Move* GameStateSpace::getBufferedMove(cadiaplayer::play::Move* move)
{
	Move* temp = NULL;
	ZKEY key = getZKey(move);
	ActionBuffer::iterator mit = m_actionbuffer.find(key);
	if(mit == m_actionbuffer.end())
	{
		temp = new Move(*move, m_moveInitQ);
		m_actionbuffer[key] = temp;
	}
	else
		temp = (*mit).second;
	return temp;
}
/*cadiaplayer::play::Move* GameStateSpace::getBufferedMove(cadiaplayer::play::parsing::Compound* compound)
{
	Move* temp = NULL;
	ZKEY key = getZKey(compound);
	ActionBuffer::iterator mit = m_actionbuffer.find(key);
	if(mit == m_actionbuffer.end())
	{
		temp = new Move(*compound, m_theory->getSymbols(), true);
		m_actionbuffer[key] = temp;
	}
	else
		temp = (*mit).second;
	return temp;
}*/
State* GameStateSpace::lookupState(unsigned int depth, ZKEY key)
{
	if(depth >= m_statespace.size())
		return NULL;
	StateMap* stateMap = m_statespace[depth];
	StateMap::iterator stateIt = stateMap->find(key);
	if(stateIt == stateMap->end())
		return NULL;
	State* state = (*stateIt).second;
	return state;
}
State* GameStateSpace::lookupAndCreateState(unsigned int depth, ZKEY key, GameState*)
{
	// If a statemap for this depth has not been generated enlarge the statespace.
	while(depth >= m_statespace.size())
	{
		m_statespace.push_back(new StateMap());
	}
	// Find the correct statemap for this depth
	StateMap* stateMap = m_statespace[depth];
	// If state has not yet been seen on this depth create a handler for it.
	State* state;
	StateMap::iterator stateIt = stateMap->find(key);
	if(stateIt == stateMap->end())
	{
		state = new State(key);
		(*stateMap)[key] = state;
#ifdef LOG_UCTTREE
		m_treelog.createNodes(depth, theory->getParentState());
#endif
	}
	else
	{
		state = (*stateIt).second;
	}
	return state;
}
QNode* GameStateSpace::lookupAction(State* state, ZKEY key)
{	
	ActionMap::iterator actionIt = state->actions.find(key);
	if(actionIt == state->actions.end())
		return NULL;
	return (*actionIt).second;	
}
QNode* GameStateSpace::lookupAndCreateAction(State* state, Move* move, ZKEY key)
{
	// Has this move been seen yet for this gamestate?
	QNode* a;
	ActionMap::iterator actionIt = state->actions.find(key);
	if(actionIt == state->actions.end())
	{
		Move* temp = NULL;
		ActionBuffer::iterator buf = m_actionbuffer.find(key);
		if(buf == m_actionbuffer.end())
		{
			temp = new Move(*move, m_moveInitQ);
			m_actionbuffer[key] = temp;
		}
		else
			temp = (*buf).second;
		a = new QNode(temp, state, m_creationCounter++);
		state->actions[key] = a;
	}
	else	
		a = (*actionIt).second;
	return a;
}

void GameStateSpace::buildStateSpace(void)
{
	// If the class is used then at least depth 0 will be used
	// save time later.
	m_statespace.push_back(new StateMap());
}

std::string GameStateSpace::getStats(void)
{
	char buffer[4096];
	std::string str = "Statespace statistics:\n-----------------------\n";
	unsigned int depth = m_statespace.size();
	unsigned int states = 0;
	unsigned int actions = m_actionbuffer.size();
	for(StateSpace::iterator dit = m_statespace.begin() ; dit != m_statespace.end() ; dit++)
	{
		states += (*dit)->size();
		
	}
	sprintf(buffer, "Depth   : %u \n", depth);
	str += buffer;	
	sprintf(buffer, "States  : %u \n", states);
	str += buffer;	
	sprintf(buffer, "Actions : %u \n", actions);
	str += buffer;	
	str += "-----------------------\n";
	return str;
}
std::string GameStateSpace::getDetails(void)
{
	char buffer[16556];
	std::string str = "Statespace details:\n-------------------\n";
	int depth = 0;
	for(StateSpace::iterator dit = m_statespace.begin() ; dit != m_statespace.end() ; dit++)
	{
		sprintf(buffer, "Depth %d: \n", depth++);
		str += buffer;
		for(StateMap::iterator sit = (*dit)->begin() ; sit != (*dit)->end() ; sit++)
		{
			sprintf(buffer, "\tstate [ZKEY:%010u-%010u]: \n", ((unsigned int)(*sit).first),((unsigned int)((*sit).first >> 32)));
			str += buffer;
			for(ActionMap::iterator ait = (*sit).second->actions.begin() ; ait != (*sit).second->actions.end() ; ait++)
			{
				sprintf(buffer, "\t\tn=%u\tq=%03.4f\t%s\t[ZKEY:%010u-%010u]\n", 
						(*ait).second->n,
						(*ait).second->q,
						m_actionbuffer[(*ait).first]->compound.toString(m_theory->getSymbols()).c_str(),
						((unsigned int)(*ait).first),
						((unsigned int)((*ait).first >> 32))
						);
				str += buffer;
			}
		}
	}
	str += "-------------------\n";
	str += "Action buffer\n";
	str += "-------------------\n";
	for(ActionBuffer::iterator bit = m_actionbuffer.begin() ; bit != m_actionbuffer.end() ; bit++)
	{
		str += (*bit).second->compound.toString(m_theory->getSymbols()).c_str();
		str += "\n";
	}
	str += "-------------------\n";
	return str;
}

std::string GameStateSpace::getActionBufferInfo(void)
{
	std::stringstream str;
	str << "-------------------\n";
	str << "Action buffer\n";
	str << "-------------------\n";
	for(ActionBuffer::iterator bit = m_actionbuffer.begin() ; bit != m_actionbuffer.end() ; bit++)
	{
		str << (*bit).second->compound.toString(m_theory->getSymbols()).c_str();
		str << "\t";
		str << bit->second->estimateQ();
		str << " (";
		str << bit->second->getN();
		str << ")\n";
	}
	str << "-------------------\n";
	return str.str();
}
std::string GameStateSpace::generateActionInfo(GameState* s, unsigned int depth)
{
	std::stringstream ss;
	ZKEY statekey = getZKey(s);
	State* state = lookupState(depth, statekey);
	if(state == NULL)
		return NULL;
	for(ActionMap::iterator a = state->actions.begin(); a != state->actions.end() ; a++)
	{
		ss << m_actionbuffer[(*a).first]->compound.toString(m_theory->getSymbols()) << "\tn=" << (*a).second->n << "\tq=" << (*a).second->q << std::endl;
	}
	return ss.str();
}
std::string GameStateSpace::stateActionSizeInfo(void)
{
	std::stringstream ss;
	long scount = 0;
	long acount = 0;
	double avg = 0.0;
	for(StateSpace::iterator dit = m_statespace.begin() ; dit != m_statespace.end() ; dit++)
	{
		for(StateMap::iterator sit = (*dit)->begin() ; sit != (*dit)->end() ; sit++)
		{
			scount++;
			for(ActionMap::iterator ait = (*sit).second->actions.begin() ; ait != (*sit).second->actions.end() ; ait++)
			{
				acount++;
				avg += 1.0/((double)acount)*(sizeof(QNode)-avg);
			}
		}
	}
	ss << "Number of states    : " << scount << std::endl;
	ss << "Num state-actions   : " << acount << std::endl;
	ss << "Avg bytes of s-a    : " << avg << std::endl;
	ss << "Total kbytes        : " << (avg/1024.0)*acount << std::endl;
	return ss.str();
}

std::string GameStateSpace::actionBufferSizeInfo(void)
{
	std::stringstream ss;
	long count = 0;
	double avg = 0.0;
	long instsize = 0;
	for(ActionBuffer::iterator bit = m_actionbuffer.begin() ; bit != m_actionbuffer.end() ; bit++)
	{
		count++;
		instsize =	sizeof(*((*bit).second));
		instsize += (*((*bit).second)).compound.sizeInBytes();
		avg += 1.0/((double)count)*(instsize-avg);
	}
	ss << "Number of actions   : " << count << std::endl;
	ss << "Avg bytes of action : " << avg << std::endl;
	ss << "Total kbytes        : " << (avg/1024.0)*count << std::endl;
	return ss.str();
}
