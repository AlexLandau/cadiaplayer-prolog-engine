#include "uctplayer.h"
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

UCTPlayer::UCTPlayer() : SelectionPlayer(), 
m_c(DEFAULT_C), 
m_useUAU(false),
m_scoreUAU(GOAL_DRAW)
{
}

UCTPlayer::~UCTPlayer()
{
}

bool UCTPlayer::adjustArrays(std::size_t branch)
{
	if(m_randmax.size() >= branch)
		return false;
	
	m_randmax.resize(branch);
	m_randexp.resize(branch);
	return true;
}

double UCTPlayer::uct(MTAction* node)
{
	double t = static_cast<double>(node->state->n); 
	double s = static_cast<double>(node->n);
	// return average + uct bonus
	return node->q + m_cValues[m_workrole]*sqrt((log(t)/s));
}

void UCTPlayer::enableUnexploredActionUrgency(double score)
{
	m_useUAU = true;
	m_scoreUAU = score;
};
void UCTPlayer::disableUnexploredActionUrgency()
{
	m_useUAU = false;
};

void UCTPlayer::newGame(cadiaplayer::play::GameTheory& theory)
{
	SelectionPlayer::newGame(theory);
	
	//std::cerr << "Using C parameter with value " << m_c << std::endl;
	m_cValues.clear();
	m_cValues.resize(theory.getRoles()->size(), m_c);
}

void UCTPlayer::setCValues(double c)
{
	m_c = c;
	for(int n = 0 ; n < m_cValues.size() ; n++)
	{
		m_cValues[n] = m_c;
	}
}


size_t UCTPlayer::selectAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	if(moves.size() == 1)
		return 0;
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTAction* node;
	
	m_action = 0;
	m_val = NEG_INF;
	m_curVal = NEG_INF;
	m_foundmax = 0;
	m_foundexp = 0;
	
	adjustArrays(moves.size());
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		// Get the state-action node from the tree if available
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		// Notify that this action is available/possible at this junction in the simulation
		// (For inheriting players)
		actionAvailable(theory, moves[n], node, t, m_workrole);
		// Get the node selection value depending on if this is and unexplored node or not
		if(node == NULL || node->n == 0)
		{
			m_randexp[m_foundexp++] = n;
			continue;
		}
		
		m_curVal = selectionNodeValue(theory, state, node);
		// Keep track of the most interesting nodes in case of needing to break a tie
		if(m_curVal == m_val)
			m_randmax[m_foundmax++] = n;
		else if(m_curVal > m_val)
		{
			// Better selection value found reset tie breaking scheme
			m_val = m_curVal;
			m_action = n;
			m_foundmax=1;
			m_randmax[0] = n;
		}
	}
	// Should we explore
	if(!m_foundmax)
		m_action = selectRandomUnexplored();
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
		m_action = selectRandomUnexplored();
	else if(m_foundmax > 1) // Is tie breaking needed
		m_action = tiebreakExploited();

	return m_action;
}

size_t UCTPlayer::selectSingleAction(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleMoves& moves)
{
	unsigned int t = getDepth(theory);
	MTState* state = m_multitree->lookupState(theory.getStateRef(), t);
	MTNode* actionNode = NULL;
	if(moves.size() == 1)
	{
		actionNode = state ? m_multitree->lookupSingleNode(state, moves[0]) : NULL;
		if(actionNode && actionNode->expanded)
			state->expanded = true;
		return 0;
	}
	MTAction* node;
	
	m_action = 0;
	m_val = NEG_INF;
	m_curVal = NEG_INF;
	m_foundmax = 0;
	m_foundexp = 0;
	m_expanded = true;
	
	adjustArrays(moves.size());
	
	for(size_t n = 0 ; n < moves.size() ; n++)
	{
		// Get the state-action node from the tree if available
		node = m_multitree->lookupAction(m_workrole, state, moves[n]);
		// Notify that this action is available/possible at this junction in the simulation
		// (For inheriting players)
		actionAvailable(theory, moves[n], node, t, m_workrole);
		// Check if every child has been fully expanded/explored to a leaf
		actionNode = state ? m_multitree->lookupSingleNode(state, moves[n]) : NULL;
		m_expanded = actionNode ? (m_expanded && actionNode->expanded) : false;
		// Get the node selection value depending on if this is and unexplored node or not
		if(node == NULL || node->n == 0)
		{
			m_randexp[m_foundexp++] = n;
			continue;
		}
		
		m_curVal = (actionNode && actionNode->expanded) ? 0 : selectionNodeValue(theory, state, node);
		// Keep track of the most interesting nodes in case of needing to break a tie
		if(m_curVal == m_val)
			m_randmax[m_foundmax++] = n;
		else if(m_curVal > m_val)
		{
			// Better selection value found reset tie breaking scheme
			m_val = m_curVal;
			m_action = n;
			m_foundmax=1;
			m_randmax[0] = n;
		}
	}
	if(!m_foundmax) 
		m_action = selectRandomUnexplored();
	else if(m_foundexp && m_val < unexploredNodeValue(theory, state, moves, m_foundexp))
		m_action = selectRandomUnexplored();
	else
		m_action = tiebreakExploited();
	
	// If all child state fully expanded -> mark this as expanded
	if(m_expanded)
		state->expanded = true;
	
	return m_action;
}
	   
void UCTPlayer::tuneCValues(double maxAvailable)
{
	if(maxAvailable>100)
		m_cValues[m_workrole] = 0.0;
	else if(maxAvailable < 10.0)
		m_cValues[m_workrole] = -4.0*maxAvailable + 80.0;
	else if(maxAvailable > 90.0)
		m_cValues[m_workrole] = -4.0*maxAvailable + 400.0;
	else 
		m_cValues[m_workrole] = m_c;
}

double UCTPlayer::selectionNodeValue(cadiaplayer::play::GameTheory& /*theory*/, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::utils::MTAction* node)
{
	return uct(node);
}

double UCTPlayer::unexploredNodeValue(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::utils::MTState* state, cadiaplayer::play::RoleMoves& moves, std::size_t unexplored)
{
	if(!isUnexploredActionUrgencyEnabled() || !state || state->n < 2)
		return SelectionPlayer::unexploredNodeValue(theory, state, moves, unexplored);;
	
	double fmuFactor = static_cast<double> (unexplored) / static_cast<double> (moves.size());
	return m_scoreUAU + m_cValues[m_workrole]*std::sqrt(log(state->n))*fmuFactor;
}
std::string UCTPlayer::getSetupInfo()
{
	std::stringstream ss;
	ss << SelectionPlayer::getSetupInfo();
	if(isUnexploredActionUrgencyEnabled())
		ss << ", " << UAU_ACTIVE_LOG_MESSAGE << "(" << m_scoreUAU << ")";
	else
		ss << ", " << UAU_INACTIVE_LOG_MESSAGE;
	return ss.str();
}
std::string UCTPlayer::getLastPlayInfo(cadiaplayer::play::GameTheory& theory)
{
	std::stringstream ss;
	ss << SelectionPlayer::getLastPlayInfo(theory);
	if(isUnexploredActionUrgencyEnabled())
		ss << UAU_ACTIVE_LOG_MESSAGE << "(" << m_scoreUAU << ")";
	else
		ss << UAU_INACTIVE_LOG_MESSAGE;
	ss << std::endl;
	return ss.str();
}
