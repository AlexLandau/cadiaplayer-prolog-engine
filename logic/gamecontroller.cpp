/*
 *  gamecontroller.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 7/11/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "gamecontroller.h"

using namespace cadiaplayer::logic;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;

GameController::GameController()
{
}
GameController::~GameController() 
{
}
bool GameController::init( cadiaplayer::play::GameTheory* gametheory, std::string gamename )
{
	m_theory = gametheory;
	m_gamename = gamename;
	m_mute = false;
	return true;
}

void GameController::make(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move )
{
	cadiaplayer::play::PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	make(pm);
}
void GameController::makeByForce(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move )
{
	PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	if(!isMute())
		return make(pm);
	overrideMute(false);
	make(pm);
	overrideMute(true);
}
void GameController::makeRelaxed(cadiaplayer::play::PlayMoves& /*moves*/)
{
	std::cerr << "GameController::makeRelaxed(PlayMoves) not implemented!" << std::endl;
}
void GameController::makeNullMove( cadiaplayer::play::GameState& state ) {
	std::cerr << "GameController::makeNullMove not implemented!" << std::endl;
};
void GameController::makeExploratory( cadiaplayer::play::RoleIndex /*role*/, cadiaplayer::play::Move* /*move*/, cadiaplayer::play::GameState& /*state*/ ) {
	std::cerr << "GameController::makeExploratory(RoleIndex, Move*, GameState&) not implemented!" << std::endl;
}

void GameController::peekNextState(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& nextState) {
	cadiaplayer::play::PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	peekNextState(pm, nextState);
}
void GameController::peekNextState(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::GameState& nextState) {
	make(moves);
	getCurrentState(nextState);
	retract();
}

void GameController::effectsPlus(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::parsing::CompoundList& effects ) {
	cadiaplayer::play::PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	effectsPlus(pm, effects);
}
void GameController::effectsPlus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects ) {
	std::cerr << "GameController::effectsPlus(PlayMoves) not implemented!" << std::endl;
}
void GameController::effectsMinus(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::parsing::CompoundList& effects ) {
	cadiaplayer::play::PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	effectsMinus(pm, effects);
}
void GameController::effectsMinus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects ) {
	std::cerr << "GameController::effectsMinus(PlayMoves) not implemented!" << std::endl;
}
void GameController::commitEffectsPlus() {
	std::cerr << "GameController::commitEffectsPlus() not implemented!" << std::endl;
}
void GameController::discardEffectsPlus() {
	std::cerr << "GameController::discardEffectsPlus() not implemented!" << std::endl;
}
void GameController::storeCurrentStateAs(cadiaplayer::play::GameStateID /*sid*/) {
	std::cerr << "GameController::storeCurrentStateAs(GameStateID) not implemented!" << std::endl;
}
void GameController::jumpToStoredState(cadiaplayer::play::GameStateID /*sid*/) {
	std::cerr << "GameController::jumpToStoredState(GameStateID) not implemented!" << std::endl;
}
void GameController::deleteStoredState(cadiaplayer::play::GameStateID /*sid*/) {
	std::cerr << "GameController::deleteStoredState(GameStateID) not implemented!" << std::endl;
}

void GameController::retract( void ) {
	std::cerr << "GameController::retract() not implemented!" << std::endl;
}
void GameController::retractByForce( void ) {
	std::cerr << "GameController::retractByForce() not implemented!" << std::endl;
}
void GameController::muteRetract( void ) 
{	
	m_mute = true;
}
void GameController::syncRetract( void ) 
{
	m_mute = false;
};

void GameController::gotoStart() {
	std::cerr << "GameController::gotoStart() not implemented!" << std::endl;
};
void GameController::gotoState(const cadiaplayer::play::StateFacts& /*state*/) {
	std::cerr << "GameController::gotoState(StateFacts) not implemented!" << std::endl;
};
void GameController::gotoStateByForce(const cadiaplayer::play::StateFacts& /*state*/) {
	std::cerr << "GameController::gotoStateByForce(StateFacts) not implemented!" << std::endl;
};
/**
 * save the current state (for retract) and set the given state as new current state
 */
void GameController::syncState(const cadiaplayer::play::StateFacts& /*state*/) {
	std::cerr << "GameController::syncState() not implemented!" << std::endl;
};
cadiaplayer::play::StateFactID GameController::getStateFact(cadiaplayer::play::parsing::Compound* /*comp*/) {
	std::cerr << "GameController::getStateFact(Compound*) not implemented!" << std::endl;
	return 0;
};
void GameController::getStateFacts(const cadiaplayer::play::GameState& state, cadiaplayer::play::StateFacts& stateFacts)
{
	for (size_t n = 0 ; n < state.size() ; ++n)
	{
		stateFacts.push_back(getStateFact(state[n]));
	}
};
std::string GameController::stateFactToString(const cadiaplayer::play::StateFactID& /*stateFact*/) {
	std::cerr << "GameController::stateFactToString(StateFactID) not implemented!" << std::endl;
	return "n/a";
};
// Assert one statefact into the current state.
void GameController::assertStateFact(const cadiaplayer::play::StateFactID& /*stateFact*/) {
	std::cerr << "GameController::assertStateFact(StateFactID) not implemented!" << std::endl;
};
// Retract one statefact from the current state.
void GameController::retractStateFact(const cadiaplayer::play::StateFactID& /*stateFact*/) {
	std::cerr << "GameController::retractStateFact(StateFactID) not implemented!" << std::endl;
};
// Recalculate the fact id when enclosed in a has_seen predicate.
cadiaplayer::play::StateFactID GameController::getHasSeenID(const cadiaplayer::play::StateFactID& stateFact, unsigned long long salt) {
	return stateFact^salt;
};

double GameController::incGoals( cadiaplayer::play::RoleIndex role ) {
	return goal(role);
};
double GameController::allGoals( cadiaplayer::play::RoleIndex role ) {
	return goal(role);
};
bool GameController::reachedGoal( cadiaplayer::play::RoleIndex role, int goal) {
	return static_cast<int>(this->goal(role))>=goal;
};

void GameController::ask(cadiaplayer::play::parsing::Compound* /*c*/, bool /*interupt*/, cadiaplayer::play::KBaseList& /*answer*/) {
	std::cerr << "GameController::ask(Compound*,bool,KBaseList) not implemented!" << std::endl;
};
size_t GameController::solutionCardinality(std::string /*predicate*/, std::string /*atom*/, size_t /*pos*/, size_t /*size*/) {
	std::cerr << "GameController::solutionCardinality(string,string,size_t,size_t) not implemented!" << std::endl;
	return 0;
};

// String interface implementation
std::string GameController::getGameFileName() {
	std::cerr << "GameController::getGameFileName() not implemented!" << std::endl;
	return "";
};
std::string GameController::getStringState() {
	std::cerr << "GameController::getStringState() not implemented!" << std::endl;
	return "";
};
void GameController::gotoStringState(std::string state) {
	std::cerr << "GameController::gotoStringState(string) not implemented!" << std::endl;
};
