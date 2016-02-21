/*
 *  breakthroughcontroller.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 6/18/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "breakthroughcontroller.h"

using namespace cadiaplayer::logic;
using namespace cadiaplayer::logic::games;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;

BreakthroughController::BreakthroughController() : GameController() 
{
	m_symbols = NULL;
	m_breakthrough = NULL;
	m_noopWhite = NULL;
	m_noopBlack = NULL;
}

BreakthroughController::~BreakthroughController() 
{
	if(m_breakthrough)
		delete m_breakthrough;
	m_factmap.clear();
	for(int n = 0 ; n < m_omnistate.size() ; n++)
	{
		if(m_omnistate[n])
			delete m_omnistate[n];
	}
	m_omnistate.clear();
	for(MovePtrStoreItr itr = m_omniwhitemoves.begin() ; itr != m_omniwhitemoves.end() ; itr++)
	{
		if(itr->second)
			delete itr->second;
	}
	m_omniwhitemoves.clear();
	for(MovePtrStoreItr itr = m_omniblackmoves.begin() ; itr != m_omniblackmoves.end() ; itr++)
	{
		if(itr->second)
			delete itr->second;
	}
	m_omniblackmoves.clear();
	if(m_noopWhite)
		delete m_noopWhite;
	if(m_noopBlack)
		delete m_noopBlack;
	
	m_gamestack.clear();
};


cadiaplayer::play::Move* BreakthroughController::moveToGDL(BTMove& move, BTPiece turn)
{
	BTMove mask = 1;
	BTMove temp = move;
	char firstcol = 'a';
	int firstrow = 0;
	char secondcol = 'a';
	int secondrow = 0;
	int i;
	for(i = 0 ; i < 64 ; i++)
	{
		if(temp & mask)
		{
			firstcol = i%8;
			firstrow = i/8;
			break;
		}
		temp = temp >> 1;
	}
	temp = temp >> 1;
	for(i+=1; i < 64 ; i++)
	{
		if(temp & mask)
		{
			secondcol = i%8; 
			secondrow = i/8;
			break;
		}
		temp = temp >> 1;
	}
	Compound* c = new Compound();
	c->setName(m_symbols->lookup(BTC_MOVE)->getId());
	c->setType(ct_COMPOUND);
	c->setOperator(co_PREDICATE);
	if(turn == BT_WHITE_PAWN)
	{
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_WHITE)->getId(),  co_ATOM));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[firstcol])->getId(),  co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[firstrow])->getId(),  co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[secondcol])->getId(), co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[secondrow])->getId(), co_INT));
	}
	else
	{
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_BLACK)->getId(),  co_ATOM));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[secondcol])->getId(), co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[secondrow])->getId(), co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[firstcol])->getId(),  co_INT));
		c->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_NUM[firstrow])->getId(),  co_INT));
	}
	Move* m = new Move(*c);
	//std::cerr << BTState::moveToString(move, *m_breakthrough) << " # " << m->toString(m_symbols) << std::endl;
	delete c;
	return m;
}

void BreakthroughController::loadGDL(cadiaplayer::play::GameTheory* t)
{
	m_omnistate.clear();
	m_omnistate.resize(130, 0); // 64 white cells, 64 black cells, 2 control
	
	m_symbols = t->getSymbols();
	Compound* pred = new Compound();
	pred->setOperator(co_PREDICATE);
	pred->setType(ct_COMPOUND);
	Compound* arg1 = new Compound();
	arg1->setOperator(co_ATOM);
	arg1->setType(ct_VALUE);
	Compound* arg2 = new Compound();
	arg2->setOperator(co_ATOM);
	arg2->setType(ct_VALUE);
	Compound* arg3 = new Compound();
	arg3->setOperator(co_ATOM);
	arg3->setType(ct_VALUE);
	
	// Noop move
	Compound* noopcomp = new Compound();
	noopcomp->setName(m_symbols->lookup(BTC_NOOP)->getId());
	noopcomp->setType(ct_VALUE);
	noopcomp->addArgument(new Compound(ct_VALUE, m_symbols->lookup(BTC_WHITE)->getId(),  co_ATOM));
	m_noopWhite = new Move(*noopcomp);
	noopcomp->getArgument(0)->setName(m_symbols->lookup(BTC_BLACK)->getId()); 
	m_noopBlack = new Move(*noopcomp);
	delete noopcomp;
	
	// generate control predicates
	pred->addArgument(arg1);
	pred->setName(m_symbols->lookup(BTC_CONTROL)->getId());
	arg1->setName(m_symbols->lookup(BTC_WHITE)->getId());
	m_omnistate[128] = new Compound(*pred);
	arg1->setName(m_symbols->lookup(BTC_BLACK)->getId());
	m_omnistate[129] = new Compound(*pred);
	
	// generate pieces on cells and moves
	pred->setName(m_symbols->lookup(BTC_CELL)->getId());
	arg1->setOperator(co_INT);
	arg2->setOperator(co_INT);
	pred->addArgument(arg2);
	pred->addArgument(arg3);
	Move* movetemp;
	BTState movegen;
	for(int i = 0 ; i < 8 ; i++)
	{
		for(int j = 0 ; j < 8 ; j++)
		{
			if(i < 7)
			{
				movegen.clear();
				movegen.setSquare(j, i, BT_WHITE_PAWN);
				m_moves.clear();
				movegen.setTurn(BT_WHITE_PAWN);
				movegen.getMoves(m_moves);
				
				for(int n = 0 ; n < m_moves.size() ; n++)
				{
					movetemp = moveToGDL(m_moves[n], BT_WHITE_PAWN);
					//std::cerr << "w: " << m_moves[n] << " [" << getTheory()->getGameStateID(movetemp) << "] " << movetemp->toString(m_symbols) << std::endl;
					//std::cerr << ">: " << games::BTState::moveToPlayString(m_moves[n], BT_WHITE_PAWN) << " -> " << movetemp->toString(m_symbols) << std::endl;
					m_omniwhitemoves[m_moves[n]] = movetemp;
					m_omnibtmoves[getTheory()->getGameStateID(movetemp)] = m_moves[n];
				}
			}
			if(i > 0)
			{
				movegen.clear();
				movegen.setSquare(j, i, BT_BLACK_PAWN);
				m_moves.clear();
				movegen.setTurn(BT_BLACK_PAWN);
				movegen.getMoves(m_moves);
				
				for(int n = 0 ; n < m_moves.size() ; n++)
				{
					movetemp = moveToGDL(m_moves[n], BT_BLACK_PAWN);
					//std::cerr << "b: " << m_moves[n] << " [" << getTheory()->getGameStateID(movetemp) << "] " << movetemp->toString(m_symbols) << std::endl;
					m_omniblackmoves[m_moves[n]] = movetemp;
					m_omnibtmoves[getTheory()->getGameStateID(movetemp)] = m_moves[n];
				}
			}
		}
	}
	
	for(int i = 0 ; i < 8 ; i++)
	{
		for(int j = 0 ; j < 8 ; j++)
		{
			int k = i*8+j;
			arg1->setName(m_symbols->lookup(BTC_NUM[j])->getId());
			arg2->setName(m_symbols->lookup(BTC_NUM[i])->getId());
			arg3->setName(m_symbols->lookup(BTC_WHITE)->getId());
			m_omnistate[k] = new Compound(*pred);
			arg3->setName(m_symbols->lookup(BTC_BLACK)->getId());
			m_omnistate[k+64] = new Compound(*pred);
		}		
	}
	
	// generate control predicates
	m_factmap.clear();
	for(StateFactID n = 0 ; n < m_omnistate.size() ; n++)
	{
		m_factmap[t->getGameStateID(m_omnistate[n])] = n;
	}
	
	m_omnifacts.clear();
	for(int n = 0 ; n < m_omnistate.size() ; n++)
	{
		m_omnifacts.push_back(t->getGameStateID(m_omnistate[n]));
	}
	
	//clean up
	delete pred;
	// does
	// delete arg1;
	// delete arg2;
	// delete arg3;
	// for us
}

void BreakthroughController::BTStateToGameState(games::BTState& btstate, cadiaplayer::play::GameState& state)
{
	BTPiece piece;
	Compound* c = NULL;
	for(int n = 0 ; n < 64 ; n++)
	{
		int i = n%8;
		int j = n/8;
		piece = btstate.getSquare(i, j);
		if(piece == BT_WHITE_PAWN)
		{
			c = new Compound(*m_omnistate[n]);
			state.push_back(c);
				
		}
		else if(piece == BT_BLACK_PAWN)
		{
			c = new Compound(*m_omnistate[n+64]);
			state.push_back(c);
		}
	}
	if(btstate.getTurn() == BT_WHITE_PAWN)
	{
		c = new Compound(*m_omnistate[128]);
		state.push_back(c);
	}
	else
	{
		c = new Compound(*m_omnistate[129]);
		state.push_back(c);
	}
}
void BreakthroughController::BTStateToStateFacts(games::BTState& btstate, cadiaplayer::play::StateFacts& facts)
{
	BTPiece piece;
	for(int n = 0 ; n < 64 ; n++)
	{
		int i = n%8;
		int j = n/8;
		piece = btstate.getSquare(i, j);
		if(piece == BT_EMPTY_CELL)
			continue;
		if(piece == BT_WHITE_PAWN)
			facts.push_back(m_omnifacts[n]);
		else
			facts.push_back(m_omnifacts[n+64]);
	}
	if(btstate.getTurn() == BT_WHITE_PAWN)
		facts.push_back(m_omnifacts[128]);
	else
		facts.push_back(m_omnifacts[129]);
}
void BreakthroughController::StateFactsToBTState(cadiaplayer::play::StateFacts& facts, games::BTState* btstate)
{
	btstate->clear();
	for(int n = 0 ; n < facts.size() ; n++)
	{
		addStateFactToBTState(facts[n], btstate);
	}
}
void BreakthroughController::addStateFactToBTState(cadiaplayer::play::StateFactID fact, games::BTState* btstate)
{
	StateFactID mapped = 0;
	if(fact)
		mapped = m_factmap[fact];
	
	if(mapped < 64) // White
	{
		//std::cerr << "Adding white fact: (" << mapped << ") " << fact << " = " << m_omnistate[mapped]->toString(m_symbols) << std::endl;
		btstate->setSquare(mapped%8, mapped/8, BT_WHITE_PAWN);
	}
	else if(mapped < 128)
	{
		//std::cerr << "Adding black fact: (" << mapped << ") " << fact << " = " << m_omnistate[mapped]->toString(m_symbols) << std::endl;
		mapped -= 64;
		btstate->setSquare(mapped%8, mapped/8, BT_BLACK_PAWN);
	}
	else 
	{
		if(mapped == 128)
			btstate->setTurn(BT_WHITE_PAWN);
		else
			btstate->setTurn(BT_BLACK_PAWN);
	}

}
void BreakthroughController::removeStateFactFromBTState(cadiaplayer::play::StateFactID fact, games::BTState* btstate)
{
	StateFactID mapped = 0;
	if(fact)
		mapped = m_factmap[fact];
	
	if(mapped < 64) // White
	{
		btstate->setSquare(mapped%8, mapped/8, BT_EMPTY_CELL);
	}
	else if(mapped < 128)
	{
		mapped -= 64;
		btstate->setSquare(mapped%8, mapped/8, BT_EMPTY_CELL);
	}
	else 
	{
		if(mapped == 128)
			btstate->setTurn(BT_BLACK_PAWN);
		else
			btstate->setTurn(BT_WHITE_PAWN);
	}
	
}

bool BreakthroughController::init( cadiaplayer::play::GameTheory* gametheory, std::string path, std::string gamename ) 
{
	GameController::init(gametheory, gamename);
	
	// Init the board
	if(m_breakthrough)
		delete m_breakthrough;
	
	m_breakthrough = new BTState();
	m_gamestack.clear();
	m_gamestack.push_back(m_breakthrough->getID());
	
	// Load up the parser symbols
	loadGDL(getTheory());
	
	// Store a seperate init state
	m_initstate.clear();
	BTStateToGameState(*m_breakthrough, m_initstate);
	
	/*std::cerr << "****** INIT DONE ******\n";
	for(int n = 0 ; n < m_omnifacts.size() ; n++)
	{
		int i = n%8;
		int j = n/8;
		std::cerr << "(" << n << "/";
		std::cerr << m_factmap[m_omnifacts[n]];
		std::cerr << ")[" << i << "," << j << "]";
		std::cerr << " FactID:" << m_omnifacts[n] << " "; 
		std::cerr << m_omnistate[n]->toString(m_symbols) << "==";
		std::cerr << m_omnistate[m_factmap[m_omnifacts[n]]]->toString(m_symbols) << std::endl;
	}*/
	
	return true;
};
bool BreakthroughController::isTerminal( ) 
{
	return m_breakthrough->isTerminal();
};

void BreakthroughController::getMoves( cadiaplayer::play::RoleIndex role, cadiaplayer::play::RoleMoves& moves ) 
{
	// If not the player's turn just return noop 
	BTPiece turn = BT_WHITE_PAWN;
	if(role)
		turn = BT_BLACK_PAWN;
	if(m_breakthrough->getTurn() != turn)
	{
		if(!role)
			moves.push_back(new Move(*m_noopWhite));
		else
			moves.push_back(new Move(*m_noopBlack));
		return;
	}
	
	// Load the available moves
	m_moves.clear();
	
	//std::cerr << m_breakthrough->toString() << std::endl;
	m_breakthrough->getMoves(m_moves);
	
	if(m_moves.empty())
		std::cerr << "Empty move set for side to move (" << (role?"white)\n":"black)\n"); 
	
	// Convert the moves to rolemoves
	Move* move;
	if(role == BT_WHITE_PAWN)
	{
		for(int n = 0 ; n < m_moves.size() ; n++)
		{
			//std::cerr << BTState::moveToString(m_moves[n], *m_breakthrough) << std::endl;
			
			move = m_omniwhitemoves[m_moves[n]];
			moves.push_back(new Move(*move));	
			
			/*if(m[n])
				std::cerr << "white:" << m[n]->toString(m_symbols) << " as " << m_omniwhitemoves[m_moves[n]]->toString(m_symbols) << std::endl;
			else
			 	std::cerr << "white:pushing NULL\n";
			 */
		}
	}
	else
	{
		for(int n = 0 ; n < m_moves.size() ; n++)
		{
			
			 
			move = m_omniblackmoves[m_moves[n]];
			moves.push_back(new Move(*move));
			
			/*if(m[n])
				std::cerr << "black:" << m[n]->toString(m_symbols) << std::endl;
			else
				std::cerr << "black:pushing NULL\n";
			*/
		}
	}
};

void BreakthroughController::getInitialState(cadiaplayer::play::GameState& state) 
{
	state.insert(state.end(), m_initstate.begin(), m_initstate.end());
}
void BreakthroughController::getCurrentState(cadiaplayer::play::GameState& state) 
{
	BTStateToGameState(*m_breakthrough, state);
}
void BreakthroughController::make(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move )
{
	cadiaplayer::play::PlayMoves pm;
	m_theory->generateRandomMove(role, move, pm);
	make(pm);
}
void BreakthroughController::makeByForce(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move )
{
	make(role, move);
}
void BreakthroughController::make(cadiaplayer::play::PlayMoves& moves)
{
	m_state.clear();
	Compound* c;
	
	if(m_breakthrough->getTurn() == BT_WHITE_PAWN)
		c = &(moves[0]->compound);
	else
		c = &(moves[1]->compound);

	GameStateID id = getTheory()->getGameStateID(c);
	BTMove move = m_omnibtmoves[id];
	m_gamestack.push_back(m_breakthrough->getID());
	m_breakthrough->make(move);
}
void BreakthroughController::makeRelaxed(cadiaplayer::play::PlayMoves& moves)
{
	std::cerr << "GameController::makeRelaxed(PlayMoves) not implemented!" << std::endl;
}
void BreakthroughController::makeNullMove( cadiaplayer::play::GameState& state ) 
{
	m_state.clear();
	
	if(m_breakthrough->getTurn() == BT_WHITE_PAWN) 
		m_breakthrough->setTurn(BT_BLACK_PAWN);
	else
		m_breakthrough->setTurn(BT_WHITE_PAWN);
	
	m_gamestack.push_back(m_breakthrough->getID());
	m_breakthrough->make(NullMove);
}
void BreakthroughController::makeExploratory( cadiaplayer::play::RoleIndex /*role*/, cadiaplayer::play::Move* /*move*/, cadiaplayer::play::GameState& /*state*/ ) 
{
	std::cerr << "GameController::makeExploratory(RoleIndex, Move) not implemented!" << std::endl;
}

void BreakthroughController::peekNextState(cadiaplayer::play::RoleIndex role, cadiaplayer::play::Move* move, cadiaplayer::play::GameState& nextState) 
{
	make(role, move);
	getCurrentState(nextState);
	retract();
}
void BreakthroughController::peekNextState(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::GameState& nextState) 
{
	return (m_breakthrough->getTurn() == BT_WHITE_PAWN) ? peekNextState(0, moves[1], nextState) : peekNextState(1, moves[0], nextState);
}
void BreakthroughController::effectsPlus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects) 
{
	std::cerr << "GameController::effectsPlus(PlayMoves) not implemented!" << std::endl;
}
void BreakthroughController::effectsMinus(cadiaplayer::play::PlayMoves& moves, cadiaplayer::play::parsing::CompoundList& effects) 
{
	std::cerr << "BreakthroughController::effectsMinus(PlayMoves) not implemented!" << std::endl;
}
void BreakthroughController::commitEffectsPlus() 
{
	std::cerr << "BreakthroughController::commitEffectsPlus() not implemented!" << std::endl;
};
void BreakthroughController::discardEffectsPlus() 
{
	std::cerr << "BreakthroughController::discardEffectsPlus() not implemented!" << std::endl;
};
void BreakthroughController::storeCurrentStateAs(cadiaplayer::play::GameStateID /*sid*/) 
{
	std::cerr << "GameController::storeCurrentStateAs(GameStateID) not implemented!" << std::endl;
}
void BreakthroughController::jumpToStoredState(cadiaplayer::play::GameStateID sid) 
{
	std::cerr << "GameController::jumpToStoredState(GameStateID) not implemented!" << std::endl;
}
void BreakthroughController::deleteStoredState(cadiaplayer::play::GameStateID sid) 
{
	std::cerr << "GameController::deleteStoredState(GameStateID) not implemented!" << std::endl;
}

void BreakthroughController::retract( void ) 
{
	if(isMute() || m_gamestack.size() == 0)
		return;
	m_breakthrough->retract(m_gamestack.back());
	m_gamestack.pop_back();
}
void BreakthroughController::retractByForce( void ) 
{
	if(!isMute())
		return retract();
	overrideMute(false);
	retract();
	overrideMute(true);
}
void BreakthroughController::muteRetract( void ) 
{
	GameController::muteRetract();
	m_syncState = m_breakthrough->getID();
}
void BreakthroughController::syncRetract( void ) 
{
	GameController::syncRetract();
	m_gamestack.push_back(m_syncState);
	retract();
	m_syncState.turn = BT_EMPTY_CELL;
}

void BreakthroughController::gotoStart() 
{
	if(m_breakthrough)
		delete m_breakthrough;
	m_breakthrough = new BTState();
	m_gamestack.clear();
}
void BreakthroughController::gotoState(const cadiaplayer::play::StateFacts& state) 
{
	syncState(state);
}
void BreakthroughController::gotoStateByForce(const cadiaplayer::play::StateFacts& state) 
{
	if(!isMute())
		syncState(state);
	
	overrideMute(false);
	syncState(state);
	overrideMute(true);
}
/**
 * save the current state (for retract) and set the given state as new current state
 */
void BreakthroughController::syncState(const cadiaplayer::play::StateFacts& state) 
{
	if(!isMute())
		m_gamestack.push_back(m_breakthrough->getID());
	
	m_breakthrough->clear();
	for(int n = 0 ; n < state.size() ; n++)
	{
		addStateFactToBTState(state[n], m_breakthrough);	
	}
}
cadiaplayer::play::StateFactID BreakthroughController::getStateFact(cadiaplayer::play::parsing::Compound* comp) 
{
	return getTheory()->getGameStateID(comp);
}
void BreakthroughController::getStateFacts(const cadiaplayer::play::GameState& state, cadiaplayer::play::StateFacts& stateFacts)
{
	for (size_t n = 0 ; n < state.size() ; ++n)
	{
		stateFacts.push_back(getStateFact(state[n]));
	}
}
std::string BreakthroughController::stateFactToString(const cadiaplayer::play::StateFactID& stateFact) 
{
	return m_omnistate[m_omnifacts[stateFact]]->toString(m_symbols);
}
// Assert one statefact into the current state.
void BreakthroughController::assertStateFact(const cadiaplayer::play::StateFactID& stateFact) 
{
	addStateFactToBTState(stateFact, m_breakthrough);
}
// Retract one statefact from the current state.
void BreakthroughController::retractStateFact(const cadiaplayer::play::StateFactID& stateFact) 
{
	removeStateFactFromBTState(stateFact, m_breakthrough);
}
// Recalculate the fact id when enclosed in a has_seen predicate.
cadiaplayer::play::StateFactID BreakthroughController::getHasSeenID(const cadiaplayer::play::StateFactID& stateFact, unsigned long long salt) 
{
	return stateFact^salt;
}

double BreakthroughController::goal( cadiaplayer::play::RoleIndex role ) 
{
	if(!m_breakthrough->isTerminal())
		return 0;
	int win = 0;
	if(m_breakthrough->getTurn() == BT_WHITE_PAWN)
		win = 1;
	if(role == win)
		return GOAL_MAX;
	return GOAL_MIN;
}
double BreakthroughController::incGoals( cadiaplayer::play::RoleIndex role ) 
{
	return goal(role);
}
double BreakthroughController::allGoals( cadiaplayer::play::RoleIndex role ) 
{
	return goal(role);
}
bool BreakthroughController::reachedGoal( cadiaplayer::play::RoleIndex role, int goal) 
{
	return static_cast<int>(this->goal(role))>=goal;
}

void BreakthroughController::ask(cadiaplayer::play::parsing::Compound* /*c*/, bool /*interupt*/, cadiaplayer::play::KBaseList& /*answer*/) {
	std::cerr << "BreakthroughController::ask(Compound*,bool,KBaseList) not implemented!" << std::endl;
}
size_t BreakthroughController::solutionCardinality(std::string /*predicate*/, std::string /*atom*/, size_t /*pos*/, size_t /*size*/) {
	std::cerr << "BreakthroughController::solutionCardinality(string,string,size_t,size_t) not implemented!" << std::endl;
	return 0;
}

// String interface implementation
std::string BreakthroughController::getGameFileName() {
	return "breakthrough.kif";
}
std::string BreakthroughController::getStringState() {
	std::cerr << "BreakthroughController::getStringState() not implemented!" << std::endl;
	return "";
}
void BreakthroughController::gotoStringState(std::string state) 
{
	std::cerr << "BreakthroughController::gotoStringState(string) not implemented!" << std::endl;
}

