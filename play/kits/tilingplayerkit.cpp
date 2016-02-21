/*
 *  tilingplayerkit.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 3/18/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#include "tilingplayerkit.h"

using namespace cadiaplayer::play::tiling;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::kits;

TilingPlayerKit::TilingPlayerKit() :
m_isValid(0),
m_learner(NULL),
m_void(false)
{
}
TilingPlayerKit::~TilingPlayerKit()
{
	if(m_learner)
		delete m_learner;
}


void TilingPlayerKit::newGameTilingEndSegment(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex playerindex)
{
	createLearner(theory, playerindex);
	m_isValid = false;
}

void TilingPlayerKit::newSearchTilingEndSegment(cadiaplayer::play::GameTheory& theory)
{
	if(isVoid())
		return;
	
	m_episode.clear();
	m_episode.addState(theory.getStateRef());
}

void TilingPlayerKit::atTerminalTilingEndSegment(cadiaplayer::play::GameTheory& /*theory*/, std::vector<double>& qValues)
{
	if(isVoid())
		return;
	
	m_episode.setReturns(qValues);
	size_t validFeatures = m_learner->playEpisode(m_episode, TILING_KIT_VALID_AVERAGE, TILING_KIT_VALID_SAMPLES);
	if(validFeatures > m_isValid)
		m_isValid = validFeatures;
}

void TilingPlayerKit::makeActionTilingEndSegment(cadiaplayer::play::GameTheory& theory)
{
	if(isVoid())
		return;
	
	m_episode.addState(theory.getStateRef());
}

std::string TilingPlayerKit::getLastPlayInfoTilingEndSegment(cadiaplayer::play::GameTheory& /*theory*/) 
{
	if(isVoid())
		return "Tilecoding Learner Kit is set to VOID\n";
	
	std::ostringstream s;
	s << "Tilecoding Learner Kit Info" << std::endl;
	s << "Is Valid      ? " << (isValid()?"True (":"False (") << m_isValid << ")" << std::endl;
	if(isValid())
		s << m_learner->getWeightInfo();
	//s << m_learner->getInfo();
	//s << m_learner->getQuickInfo();
	return s.str();
}

double TilingPlayerKit::getTilingMoveEvaluation(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex roleindex, cadiaplayer::play::RoleIndex playerindex, cadiaplayer::play::Move* move)
{
	/*std::cerr << "***** Tilings Info *****" << std::endl;
	std::cerr << "m_pvt     : " << m_learner->getTiles()->getInfo(m_pvt) << std::endl;
	std::cerr << "m_rptPlace: " << m_learner->getTiles()->getInfo(m_rptPlace) << std::endl;
	std::cerr << "m_rptMove : " << m_learner->getTiles()->getInfo(m_rptMove) << std::endl;
	std::cerr << "Tilings list:" << std::endl;
	for(int n = 0 ; n < m_learner->getTiles()->getTilings().size() ; n++)
	{
		std::cerr << n << ": " << m_learner->getTiles()->getInfo(m_learner->getTiles()->getTilings()[n]) << std::endl;
	}
	std::cerr << "************************" << std::endl;*/
 	
	if(!move->compound.getArguments() || isVoid())
		return TILING_KIT_IGNORE_VALUE;
	
	
#ifndef TILING_KIT_DISABLE_MARKER_POSITIONS
	double w = 0;
	cadiaplayer::play::GameStateID pos = m_rptPlace->getFeature(move, roleindex);
	if(pos == FEATURE_NONE)
		pos = m_rptMove->getFeature(move, roleindex);
		//pos = m_learner->getTiles()->getTilings()[3+roleindex]->getFeature(&(move->compound));
	
	if(pos != FEATURE_NONE)
	{
		w = TILING_KIT_MARKER_WEIGHT*m_learner->getWeight(pos);
		if(roleindex != playerindex)  
			w = -w;
	}	
	
	if(w != 0)
		return w;
	w = TILING_KIT_IGNORE_VALUE;
#else
	double w = TILING_KIT_IGNORE_VALUE;
#endif
	cadiaplayer::play::GameStateID captured = getCaptureFeature(theory, move, roleindex);
	if(!captured)
		return w;
	
	cadiaplayer::play::GameStateID moved = getMoveFeature(theory, move, roleindex);
	
	if(captured == moved)
		return w;
	
	if(moved)
		w =  2*m_learner->getWeight(captured)+m_learner->getWeight(moved);
	else
		w = m_learner->getWeight(captured);
	
	// If my turn I'm capturing an opponent piece, which is good, so invert its "being on the board" value.
	if(roleindex == playerindex)  
		w = -w;
	
	//std::cerr << "role " << roleindex << " detected capture of piece " << captured << " with piece " << moved << " giving weight " << w << std::endl;
	
	return w;
}
cadiaplayer::play::GameStateID TilingPlayerKit::getMoveFeature(GameTheory& theory, Move* move, cadiaplayer::play::RoleIndex roleindex)
{
	if(!move->compound.getArguments())
		return 0;
	cadiaplayer::play::parsing::SymbolID piece = 0;
	GameState* s = theory.getStateRef();
	cadiaplayer::play::parsing::CompoundArgs& ma = *(move->compound.getArguments());
	if(ma.size() < 4)
		return piece;
	for(size_t n = 0 ; n < s->size() ; n++)
	{
		cadiaplayer::play::parsing::Compound* temp = (*s)[n];
		cadiaplayer::play::parsing::CompoundArgs& sa = *(temp->getArguments());
		if(sa.size() < 3)
			continue;
		//std::cerr << "Checking : " << sa[0]->getName() << "==" << ma[3]->getName() << " && " << sa[1]->getName() << "==" << ma[4]->getName() << " -> " << sa[2]->getName() << std::endl;
		/*if(sa[0]->getName() == ma[ma.size()-4]->getName() && sa[1]->getName() == ma[ma.size()-3]->getName())
		{
			piece = m_learner->getTiles()->getFeature( temp );//sa[2]->getName();
			break;
		}*/
		if(sa[0]->getName() == ma[0]->getName() && sa[1]->getName() == ma[1]->getName())
		{
			piece = m_learner->getTiles()->getFeature( temp );//sa[2]->getName();
			break;
		}
	}
	return piece;
}
cadiaplayer::play::GameStateID TilingPlayerKit::getCaptureFeature(GameTheory& theory, Move* move, cadiaplayer::play::RoleIndex roleindex)
{
	if(!move->compound.getArguments())
		return 0;
	cadiaplayer::play::parsing::SymbolID piece = 0;
	GameState* s = theory.getStateRef();
	cadiaplayer::play::parsing::CompoundArgs& ma = *(move->compound.getArguments());
	if(ma.size() < 4)
		return piece;
	for(size_t n = 0 ; n < s->size() ; n++)
	{
		cadiaplayer::play::parsing::Compound* temp = (*s)[n];
		cadiaplayer::play::parsing::CompoundArgs& sa = *(temp->getArguments());
		if(sa.size() < 3)
			continue;
		//std::cerr << "Checking : " << sa[0]->getName() << "==" << ma[3]->getName() << " && " << sa[1]->getName() << "==" << ma[4]->getName() << " -> " << sa[2]->getName() << std::endl;
		if(sa[0]->getName() == ma[ma.size()-2]->getName() && sa[1]->getName() == ma[ma.size()-1]->getName())
		{
			piece = m_learner->getTiles()->getFeature( temp );//sa[2]->getName();
			break;
		}
	}
	return piece;
}

void TilingPlayerKit::createLearner(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex playerindex)
{
	if(m_learner)
		delete m_learner;
	m_learner = new TileCodingLearner(&theory, playerindex);
	m_learner->setMinParameters(3);
	
	TileCoding* coding = m_learner->getTiles();
	TileArguments ta;
	
	ta.push_back(2);
	ta.push_back(theory.getRoles()->size());
	m_pvt = new PieceValueTiling(theory.getSymbols(), ta);
	coding->addTiling(m_pvt);
	
	m_rptPlace = 0;
	ta[0]=1;
	for(size_t n = 0 ; n < theory.getRoles()->size() ; n++)
	{
		ta[1]=n;
		m_rptPlace = new RolePieceTiling(theory.getSymbols(), ta);
		coding->addTiling(m_rptPlace);
	}
	
	m_rptMove = 0;
	ta[0]=3;
	for(size_t n = 0 ; n < theory.getRoles()->size() ; n++)
	{
		ta[1]=n;
		m_rptMove = new RolePieceTiling(theory.getSymbols(), ta);
		coding->addTiling(m_rptMove);
	}
}
