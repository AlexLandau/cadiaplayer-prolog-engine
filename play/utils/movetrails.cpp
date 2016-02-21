/*
 *  movetrails.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 10/13/11.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#include "movetrails.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;

MoveSelectionData::MoveSelectionData() :
m_winIndex(0),
m_lossIndex(0)
{

}
MoveSelectionData::~MoveSelectionData()
{
	
}
void MoveSelectionData::resetMoveSelectionTrails(cadiaplayer::play::RoleIndex roleCount)
{
	if(m_roleCount != roleCount)
	{
		m_roleCount = roleCount;
		m_trails.clear();
		m_trails.resize(roleCount);
	}
	m_start = std::numeric_limits<int>::max();
	m_end = 0;
	m_winend = 0;
	m_nextcuts = 0;
	m_blanks = 0;
}
void MoveSelectionData::prepareActions(cadiaplayer::play::RoleIndex role, int depth)
{
	if(depth < m_start)
		m_start = depth;
	if(depth >= m_end)
	{
		m_end = depth+1;
#ifdef DEBUG_MOVETRAILS
		std::cerr << "m_end moved back to " << m_end << std::endl; 
#endif
		for(int n = 0 ; n < m_roleCount ; ++n)
		{
			m_trails[n].resize(m_end);	
		}
		m_mark = m_end;
		m_winend = m_end;
	}
	m_trails[role][depth].available.clear();
	m_trails[role][depth].selected = 0;
}
void MoveSelectionData::addAction(cadiaplayer::play::RoleIndex role, int depth, cadiaplayer::play::GameStateID id, cadiaplayer::play::Move* move)
{
	// Call prepare actions first to adjust structures
	//adjustStructures(role, depth);
	//m_trails[role][depth].available.insert(move);
	m_trails[role][depth].available[id] = move;
}
void MoveSelectionData::selectAction(cadiaplayer::play::RoleIndex role, int depth, cadiaplayer::play::GameStateID id, cadiaplayer::play::Move* move)
{
	// Call prepare actions first to adjust structures
	//adjustStructures(role, depth);
	m_trails[role][depth].selected = id;
	m_trails[role][depth].selectedMove = move;
}
bool MoveSelectionData::nextShortcut(cadiaplayer::play::RoleIndex turn)
{
	int pos = m_mark-1-(m_nextcuts*m_roleCount);
	if(pos <= 1)
		return false;
	m_cutpos = -1;
	GameStateID move = m_trails[turn][pos].selected;
	Move* curMove = m_trails[turn][pos].selectedMove;
#ifdef DEBUG_MOVETRAILS
	std::cerr << "Setting scanning move to " << move << " from pos " << pos << " with m_mark at " << m_mark << std::endl; 
#endif
	for (pos -= m_roleCount; pos >= m_start ; pos-=m_roleCount) 
	{
#ifdef DEBUG_MOVETRAILS
		std::cerr << "Scanning for move at pos " << pos << std::endl; 
#endif
		if(m_trails[turn][pos].available.find(move) != m_trails[turn][pos].available.end())
		{	
			m_cutpos = pos;
			m_mark -= m_roleCount;
#ifdef DEBUG_MOVETRAILS
			std::cerr << "Blanking -> pos:" << pos << " turn:" << turn << " m_roleCount:" << m_roleCount << std::endl; 
#endif
			blankBelow(pos, turn, m_roleCount);
			if(!m_nextcuts)
			{	
#ifdef DEBUG_MOVETRAILS
				std::cerr << "Moving win end marker (" << m_mark << "/" << m_end 
				<< ") from " << m_winend << " to ";
#endif
				m_winend -= m_roleCount;
#ifdef DEBUG_MOVETRAILS
				std::cerr << m_winend << std::endl;
#endif
			}
		}
		else
			break;
	}
	if(m_cutpos > -1)
	{
//		m_mark = m_cutpos+1+(m_nextcuts*m_roleCount);
//#ifdef DEBUG_MOVETRAILS
//		std::cerr << "Moving m_mark to " << m_mark << std::endl; 
		//#endif
		m_trails[turn][m_cutpos].selected = move;
		m_trails[turn][m_cutpos].selectedMove = curMove;
#ifdef DEBUG_MOVETRAILS
		std::cerr << "Moving selected at cutpos " << m_cutpos << " to " << move << " with mark " << m_mark << std::endl; 
#endif
	}
//	else
//		m_mark -= m_roleCount;
	m_nextcuts++;
	return true;
}
int MoveSelectionData::allShortcuts(cadiaplayer::play::RoleIndex turn)
{
	int cut = 0;
	while (nextShortcut(turn)) 
	{
		if(m_cutpos > -1)
			++cut;
		//turn++;
		//turn = turn%m_roleCount;
	}
	if(m_blanks >= m_trails[turn].size()-2)
		cut = 0;
	return cut;
}
void MoveSelectionData::blankBelow(int pos, cadiaplayer::play::RoleIndex turn, int count)
{	
	m_blanks += count;
//	for(int turn = 0 ; turn < m_roleCount ; turn++)
//	{
		for( ; count > 0 ; --count)
		{
			m_trails[turn][++pos].selected = 0;
		}
//	}
}

void MoveSelectionData::initIteration()
{
	m_winIndex= m_lossIndex = m_start;
}
bool MoveSelectionData::hasNextMoveId(cadiaplayer::play::RoleIndex role)
{
	if(role == m_winRole)
		return m_winIndex < m_winend;
	else 
		return m_lossIndex < m_end;
}
GameStateID MoveSelectionData::getNextMoveId(cadiaplayer::play::RoleIndex role)
{
	if(role != m_winRole)
		return m_trails[role][m_lossIndex++].selected;
	GameStateID id;
	do 
	{
		id = m_trails[role][m_winIndex++].selected;
	}
	while (!id);
	return id;
}

void MoveSelectionData::toString(std::ostream& stream)
{
	for(int role = 0 ; role < m_trails.size() ; ++role)
	{
		stream << "Role " << (role+1) << std::endl;
		for(int trail = m_start ; trail < m_end ; ++trail)
		{
			toString(m_trails[role][trail], stream);
			stream << std::endl;
		}
	}
}
std::string MoveSelectionData::toString()
{
	std::stringstream ss;
	toString(ss);
	return ss.str();
}

