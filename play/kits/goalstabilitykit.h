/*
 *  goalstability.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 5/16/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef GOALSTABILITY_H
#define GOALSTABILITY_H

//#define DEBUG_GOALSTABILITY_H
//#define USE_OPPONENT_GOAL_BALANCING

#include <vector>
#include <sstream>
#include "../players/gameplayer.h"
#include "../../utils/statistics.h"

namespace cadiaplayer {
	namespace play {
		namespace kits {
			
			const int		GOAL_AVAILABILITY_VECTOR_SIZE		= 101;
			
			const double	DEFAULT_UNSTABLE_THRESHOLD			= 1000;
			const int		DEFAULT_SAMPLE_MINIMUM				= 100;
			const int		DEFAULT_ROLLOUT_STEPS_THRESHOLD		= 2;
			
			const int		DEFAULT_MINIMUM_TERMINAL_SPAN		= 10;
			const double	DEFAULT_TERMINAL_CUTOFF_RATIO		= 0.33;
			
			typedef std::vector<int>			GoalAvailability; 
			typedef std::vector<double>			StabilitySerie; 
			typedef std::vector<StabilitySerie>	StabilitySeries;
			
			class GoalStabilityKit
			{
			private:
				bool				m_enabled;
				int					m_imedgoals;
				int					m_current;
				int					m_calculated;
				double				m_stability;
				GoalAvailability	m_intermediate;
				StabilitySeries		m_series;
				cadiaplayer::play::players::GamePlayer* m_player;
				int					m_role;
#ifdef USE_OPPONENT_GOAL_BALANCING
				int					m_opp;
#endif
				double				m_threshold;
				int					m_rolloutsteps;
				int					m_sampleminimum;
				bool				m_onlycollectminimum;
				bool				m_goalunchanged;
				double				m_startupgoal;
				int					m_firstgoalchange;
				int					m_deepestterminal;				
				int					m_roundminimum;
				
				int					m_termgoals;
				GoalAvailability	m_terminal;
				GoalAvailability	m_terminalsAt;
				int					m_firstterminal;
				int					m_minimumterminalspan;
				int					m_terminalcalculated;
				bool				m_terminalcutoff;
				int					m_roundminimumterminal;
				
				void calculateStability();
			public:
				GoalStabilityKit();
				virtual ~GoalStabilityKit();
				
				// Segments
				void newGameGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::players::GamePlayer* player);
				void newSearchGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory);
				void makeActionGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory);
				void atTerminalGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, std::vector<double>& qValues);
				virtual bool cutoffGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory, std::size_t rolloutsteps);
				std::string getLastPlayInfoGoalStabilityEndSegment(cadiaplayer::play::GameTheory& theory);
				
				// Helper functions
				bool	isFrozen();  // No variance, not useable.
				bool	isStable();
				double	getStability();
				int		getSampleSize(){return m_calculated;};
				double	getUnstabilityThreshold(){return m_threshold;};
				void	setUnstabilityThreshold(double threshold){m_threshold = threshold;};
				int		getRolloutSteps(){return m_rolloutsteps;};
				void	setRolloutSteps(int steps){m_rolloutsteps = steps;};
				int		getSampleMinimum(){return m_sampleminimum;};
				void	setSampleMinimum(int steps){m_sampleminimum = steps;};
				bool	enabled(){return m_enabled;};
				void	setEnabled(bool enabled){m_enabled = enabled;};
				bool	onlyCollectMinimum(){return m_onlycollectminimum;};
				void	setOnlyCollectMinimum(bool only){m_onlycollectminimum = only;};
				bool	minimumReached();
				int		getIntermediateGoalsCount(){return m_imedgoals;};
				
				int		getTerminalGoalsCount(){return m_termgoals;};
				bool	isTerminalCutoff();
				int		getMinimumTerminalSpan(){return m_minimumterminalspan;};
				void	setMinimumTerminalSpan(int minimumterminalspan){m_minimumterminalspan = minimumterminalspan;};
				
				bool	isWithinMinimumSpan();
				bool	isWithinMinimumTerminalSpan();
				bool	isActive();
				
				void reset();
			private:
				void newSeries();
				void addGoal(double goal, bool terminal);
				double calculateSerieVariance(int index);
				void updateStability(double variance);
			};
		}}} // namespaces
#endif //GOALSTABILITY_H

