/*
 *  movetrails.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/24/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef MOVETRAILS_H 
#define MOVETRAILS_H

#include <ext/hash_map>
#include <vector>
//#include <set>
#include <ext/hash_map>
#include <limits>
#include "multitree.h"

//#define DEBUG_MOVETRAILS

namespace cadiaplayer
{
	namespace play 
	{
		namespace utils
		{
			// Holder move ids for RAVE
			typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, cadiaplayer::play::utils::QValue, cadiaplayer::utils::LongLongHash> MoveTrail;
			// Holder for multiple move trails
			typedef std::vector<MoveTrail*> MoveTrails;
						
			// Holder move ids for Shortcutting the simulation 
			//typedef std::set<cadiaplayer::play::GameStateID> MoveSet;
			typedef __gnu_cxx::hash_map<cadiaplayer::play::GameStateID, cadiaplayer::play::Move*, cadiaplayer::utils::LongLongHash> MoveSet;
			struct MoveSelection 
			{
				cadiaplayer::play::GameStateID selected;
				cadiaplayer::play::Move* selectedMove;
				MoveSet available;
			};
			typedef std::vector<MoveSelection> MoveSelectionTrail;
			// Holder for multiple Shortcuts
			typedef std::vector<MoveSelectionTrail> MoveSelectionTrails;
			// Holder for multiple Shortcuts
			class MoveSelectionData
			{
			private:
				MoveSelectionTrails m_trails;
				int m_start;
				int m_end;
				int m_winend;
				int m_mark;
				int m_nextcuts;
				int m_blanks;
				int m_cutpos;
				cadiaplayer::play::RoleIndex m_roleCount;
				cadiaplayer::play::RoleIndex m_winRole;
				int m_winIndex;
				int m_lossIndex;
			public:
				MoveSelectionData();
				~MoveSelectionData();
				void resetMoveSelectionTrails(cadiaplayer::play::RoleIndex roleCount);
				// Call prepareActions before adding first action and selecting one.
				void prepareActions(cadiaplayer::play::RoleIndex role, int depth);
				// Call prepareActions before first call to this in a specific state
				void addAction(cadiaplayer::play::RoleIndex role, int depth, cadiaplayer::play::GameStateID id, cadiaplayer::play::Move* move);
				void selectAction(cadiaplayer::play::RoleIndex role, int depth, cadiaplayer::play::GameStateID id, cadiaplayer::play::Move* move);
				void setWinRole(cadiaplayer::play::RoleIndex winRole){m_winRole = winRole;};
				cadiaplayer::play::RoleIndex getWinRole(){return m_winRole;};
				bool nextShortcut(cadiaplayer::play::RoleIndex turn);
				int allShortcuts(cadiaplayer::play::RoleIndex turn);
				void blankBelow(int pos, cadiaplayer::play::RoleIndex turn, int count);
				inline int getEnd(){return m_end;};
				inline int getBlanks(){return m_blanks;};
				int roleCount(){return m_trails.size();}
				int pathLength(){return m_trails[0].size();}
				GameStateID getMoveId(cadiaplayer::play::RoleIndex role, int ply){return m_trails[role][ply].selected;};
				void initIteration();
				bool hasNextMoveId(cadiaplayer::play::RoleIndex role);
				GameStateID getNextMoveId(cadiaplayer::play::RoleIndex role);
				void toString(std::ostream& stream);
				std::string toString();
				
				// toString functions
				static void toString(MoveSet& moveset, std::ostream& stream)
				{
					if(moveset.empty())
					{
						stream << "{}";	
						return;
					}
					MoveSet::iterator itr = moveset.begin();
					stream << "{" << itr->first;
					for (++itr ; itr != moveset.end() ; ++itr) 
					{
						stream << "," << itr->first;
					}
					stream << "}";
				};
				static std::string toString(MoveSet& moveset)
				{
					std::stringstream ss;
					toString(moveset, ss);
					return ss.str();
				};
				static void toString(MoveSelection& selection, std::ostream& stream)
				{
					toString(selection.available, stream);
					stream << "->" << selection.selected;
				};
				static std::string toString(MoveSelection& selection)
				{
					std::stringstream ss;
					toString(selection, ss);
					return ss.str();
				};
				static void toString(MoveSelectionTrail& trail, std::ostream& stream)
				{
					for(int n = 0 ; n < trail.size() ; ++n)
					{
						toString(trail[n], stream);
						stream << std::endl;
					}
				};
				static std::string toString(MoveSelectionTrail& trail)
				{
					std::stringstream ss;
					toString(trail, ss);
					return ss.str();
				};
				static void toString(MoveSelectionTrails& trails, std::ostream& stream)
				{
					for(int n = 0 ; n < trails.size() ; ++n)
					{
						stream << "Role " << (n+1) << std::endl;
						toString(trails[n], stream);
					}
				};
				static std::string toString(MoveSelectionTrails& trails)
				{
					std::stringstream ss;
					toString(trails, ss);
					return ss.str();
				};
			};
		}
	}
}
#endif // MOVETRAILS_H
