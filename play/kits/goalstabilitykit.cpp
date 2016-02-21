/*
 *  goalstability.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/16/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "goalstabilitykit.h"	
#include "../../utils/statistics.h"

using namespace cadiaplayer::play::kits;

GoalStabilityKit::GoalStabilityKit():
m_player(NULL),
m_threshold(DEFAULT_UNSTABLE_THRESHOLD),
m_rolloutsteps(DEFAULT_ROLLOUT_STEPS_THRESHOLD),
m_role(0),
#ifdef USE_OPPONENT_GOAL_BALANCING
m_opp(1),
#endif
m_enabled(true),
m_sampleminimum(DEFAULT_SAMPLE_MINIMUM),
m_onlycollectminimum(true),
m_roundminimum(10000),
m_roundminimumterminal(10000),
m_terminalcutoff(false),
m_firstterminal(10000),
m_terminalcalculated(10000),
m_minimumterminalspan(DEFAULT_MINIMUM_TERMINAL_SPAN)
{
}
GoalStabilityKit::~GoalStabilityKit()
{
	m_series.clear();
	m_intermediate.clear();
	m_terminal.clear();
	m_terminalsAt.clear();
}

// Segments
void GoalStabilityKit::newGameGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::players::GamePlayer* player)
{
	reset();
	m_player = player;
	if(m_player)
	{
		m_role = m_player->getRoleIndex();
#ifdef USE_OPPONENT_GOAL_BALANCING
		if(m_role)
			m_opp = 0;
#endif
	}
}
void GoalStabilityKit::newSearchGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory)
{
	if(minimumReached())
		return;
	newSeries();
	m_goalunchanged = true;
#ifdef USE_OPPONENT_GOAL_BALANCING
	m_startupgoal = theory.goal(m_role) - theory.goal(m_opp);
#else
	m_startupgoal = theory.goal(m_role);
#endif
	if(m_startupgoal < 0)
		m_startupgoal = 0.0;
	
}
void GoalStabilityKit::makeActionGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory)
{
	if(minimumReached())
		return;
#ifdef USE_OPPONENT_GOAL_BALANCING
	double goal = theory.goal(m_role) - theory.goal(m_opp);
#else
	double goal = theory.goal(m_role);
#endif
	
	if(goal < 0)
		return;
	
	if(m_goalunchanged && m_startupgoal != goal)
	{
		m_goalunchanged = false;
		m_firstgoalchange = theory.getRound();
	}
	addGoal(goal, theory.isTerminal());
}
void GoalStabilityKit::atTerminalGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues)
{
	if(!theory.isTerminal())
		return;
	//if(minimumReached())
	//	return;
#ifdef USE_OPPONENT_GOAL_BALANCING
	double goal = theory.goal(m_role) - theory.goal(m_opp);
#else
	double goal = theory.goal(m_role);
#endif
	
	int round = theory.getRound();
	if( round > m_deepestterminal)
		m_deepestterminal = round;
	if(m_firstterminal > round)
		m_firstterminal = round;
	if(m_terminalsAt.size() <= round)
		m_terminalsAt.resize(round+1, 0);
	m_terminalsAt[round]++; 
	
	if(goal < 0)
		return;
	
	/*if(m_goalunchanged && m_startupgoal != goal)
	{
		m_goalunchanged = false;
		if(round < m_firstgoalchange)
			m_firstgoalchange = round;
	}*/
		
	addGoal(goal, true);
}
bool GoalStabilityKit::cutoffGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, std::size_t rolloutsteps)
{
	if(!enabled())
		return false;
	if(m_rolloutsteps > rolloutsteps)
		return false;
	calculateStability();
	if(m_sampleminimum <= getSampleSize())
	{
		if (isWithinMinimumSpan() && isStable())
		{
			if(m_roundminimum > theory.getRound())
				return false;   
			else
			{
				//std::cerr << "Stability Cutting at " << theory.getRound() << std::endl; 
				return true;
			}
		}
	}
	if(isTerminalCutoff() && isWithinMinimumTerminalSpan())
	{
		if(m_roundminimumterminal > theory.getRound())
			return false;   
		else
		{
			//std::cerr << "Terminal Cutting at " << theory.getRound() << std::endl; 
			return true;
		}
	}
	return false;
}

std::string GoalStabilityKit::getLastPlayInfoGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory)
{
	calculateStability();
	isTerminalCutoff();
	std::stringstream ss;
	ss << "GoalStabilityKit info (cuts at " << m_rolloutsteps << ")" << std::endl;
	ss << "Sample count            : " << getSampleSize() << std::endl;
	ss << "Sample minimum          : " << getSampleMinimum() << std::endl;
	ss << "First goal depth        : " << m_firstgoalchange << std::endl;
	ss << "First terminal depth    : " << m_firstterminal << std::endl;
	ss << "Deepest terminal        : " << m_deepestterminal << std::endl;
	ss << "Game round minimum      : " << m_roundminimum << std::endl;
	ss << "Terminal round minimum  : " << m_roundminimumterminal << std::endl;
	ss << "Itermediate goal count  : " << m_imedgoals << std::endl;
	ss << "Terminal goal count     : " << m_termgoals << std::endl;
	ss << "Stability               : " << getStability() << std::endl;
	ss << "Stability threshold     : " << getUnstabilityThreshold() << std::endl;
	ss << "Is stable               : " << (isStable()?"true":"false") << std::endl;
	ss << "Is frozen               : " << (isFrozen()?"true":"false") << std::endl;
	ss << "Is terminal cutoff      : " << (isTerminalCutoff()?"true":"false") << std::endl;
	ss << "Within minumum span     : " << (isWithinMinimumSpan()?"true":"false") << std::endl;
	ss << "Within minumum terminal : " << (isWithinMinimumTerminalSpan()?"true":"false") << std::endl;
	ss << "Is active               : " << (isActive()?"true":"false") << std::endl;
	
	return ss.str();
}

// Helper functions
bool GoalStabilityKit::isFrozen() // No variance, not useable.
{
	calculateStability();
	return m_stability == 0.0;
}  
bool GoalStabilityKit::isStable()
{
	calculateStability();
	return (!isFrozen() && (m_stability < m_threshold));
}
bool GoalStabilityKit::isTerminalCutoff()
{
	int span = m_deepestterminal - m_firstterminal;
	if(m_terminalcalculated == span)
		return m_terminalcutoff;
	//std::cerr << "Checking Cutoff..." << m_termgoals << "\n";
	m_terminalcalculated = span;
	//m_terminalcutoff = false;
	if(span >= m_minimumterminalspan)
	{
		/*int n = m_firstterminal-1;
		int collected = 0;
		int limit = (int)(m_current*DEFAULT_TERMINAL_CUTOFF_RATIO);
		for(n = 0; n < m_terminalsAt.size() ; n++)
		{
			collected += m_terminalsAt[n];
			std::cerr << "collected += " << m_terminalsAt[n] << " = " << collected << std::endl; 
			if(collected > limit)
				break;
		}
		if(n < m_terminalsAt.size())
		{
			m_roundminimumterminal = m_firstterminal + n;
			m_terminalcutoff = true;
		}*/
		m_roundminimumterminal = m_firstterminal + ((int)(span*DEFAULT_TERMINAL_CUTOFF_RATIO));
		//std::cerr << "m_roundminimumterminal = " << m_roundminimumterminal << std::endl; 
		m_rolloutsteps = m_roundminimumterminal; 
		m_terminalcutoff = true;
	}
	return m_terminalcutoff;
}

bool GoalStabilityKit::isWithinMinimumSpan()
{
	return m_roundminimum < m_deepestterminal;
}
bool GoalStabilityKit::isWithinMinimumTerminalSpan()
{
	//std::cerr << "m_roundminimumterminal < m_deepestterminal = " << m_roundminimumterminal << " < " << m_deepestterminal << " = " << (m_roundminimumterminal < m_deepestterminal ? "true" : "false") << std::endl;
	return m_roundminimumterminal < m_deepestterminal;
}

bool GoalStabilityKit::isActive()
{
	return ((isStable() && isWithinMinimumSpan()) || (isTerminalCutoff() && isWithinMinimumTerminalSpan()));
}

double GoalStabilityKit::getStability()
{
	// Calculate stability;
	calculateStability();
	return m_stability;
}
void GoalStabilityKit::newSeries()
{
	++m_current;
	m_series.resize(m_current+1);
}
void GoalStabilityKit::addGoal(double goal, bool terminal)
{
	int g = (int)goal;
	if(g < 0)
		return;
		
	if(terminal)
	{
#ifdef DEBUG_GOALSTABILITY_H
		assert(m_terminal.size() > g);
#endif
		if(!m_terminal[g])
		{
			m_termgoals++;
			m_terminal[g]++;
		}
	}
	else
	{
		if(!m_intermediate[g])
		{
#ifdef DEBUG_GOALSTABILITY_H
			assert(m_intermediate.size() > g);
#endif
			m_imedgoals++;
			m_intermediate[g]++;
		}
#ifdef DEBUG_GOALSTABILITY_H
		assert(m_series.size() > m_current);
#endif		
		m_series[m_current].push_back(goal);
	}
}
void GoalStabilityKit::reset()
{
	m_current = -1;
	m_imedgoals = 0;
	m_termgoals = 0;
	m_calculated = 0;
	m_stability = 0.0;
	m_series.clear();
	m_intermediate.clear();
	m_intermediate.resize(GOAL_AVAILABILITY_VECTOR_SIZE, 0);
	m_terminal.clear();
	m_terminal.resize(GOAL_AVAILABILITY_VECTOR_SIZE, 0);
	m_terminalsAt.clear();
	m_firstgoalchange = 10000;
	m_deepestterminal = 0;
	m_terminalcutoff = false;
	m_firstterminal = 10000;
	m_terminalcalculated = 10000;
	m_roundminimum = 10000;
	m_roundminimumterminal = 10000;

}

bool GoalStabilityKit::minimumReached()
{
	return m_onlycollectminimum && (m_current >= getSampleMinimum());
}
void GoalStabilityKit::calculateStability()
{
	if(m_calculated == m_current)
		return;
	if(m_deepestterminal < m_firstgoalchange)
		m_firstgoalchange = m_deepestterminal;
	// Check if there has been any variance to justify calculations
	if(m_imedgoals < 2)
	{
		m_calculated = m_current;
		return;
	}
	
	while(m_calculated < m_current)
	{
		double var = calculateSerieVariance(m_calculated);
		updateStability(var);
	}
	m_roundminimum = m_firstgoalchange + DEFAULT_ROLLOUT_STEPS_THRESHOLD;//m_rolloutsteps;
}

double GoalStabilityKit::calculateSerieVariance(int index)
{
#ifdef DEBUG_GOALSTABILITY_H
	assert(m_series.size() > index);
#endif
	StabilitySerie& serie = m_series[index];
	StabilitySerie diff;
	double val = 0.0;
	double sum = 0.0;
	for(int n = 1 ; n < serie.size() ; ++n)
	{
		val =  serie[n] - serie[n-1];
		diff.push_back(val);
		sum += val;
	}
	return cadiaplayer::utils::variance(diff, sum);
}
void GoalStabilityKit::updateStability(double variance)
{
	++m_calculated;
	m_stability += (1.0/m_calculated)*(variance-m_stability);
}


