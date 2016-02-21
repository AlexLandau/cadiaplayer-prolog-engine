/*
 *  episode.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "episode.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::players;
using namespace cadiaplayer::play::utils;

void Episode::clear()
{
	/*for(size_t n = 0 ; n < game.size() ; ++n)
	{
		for(size_t m = 0 ; m < game[n].size() ; ++m)
		{
			delete game[n][m];
		}
		game[n].clear();
	}*/
	game.clear();
	for(size_t n = 0 ; n < deleteList.size() ; ++n)
	{
		for(size_t m = 0 ; m < deleteList[n]->size() ; ++m)
		{
			delete (*(deleteList[n]))[m];
		}
		deleteList[n]->clear();
	}
	deleteList.clear();
}

void Episode::load(string file)
{
	ifstream i;
	i.open(file.c_str());
	load(i);
	i.close();
}
void Episode::save(string file)
{
	ofstream o;
	o.open(file.c_str());
	save(o);
	o.flush();
	o.close();
}
void Episode::load(ifstream& i)
{
	clear();
	//i >> role;
	size_t rsize;
	i >> rsize;
	double reward;
	for(size_t n = 0 ; n < rsize ; ++n)
	{
		i >> reward;
		returns.push_back(reward);
	}
	size_t count;
	i >> count;
	for(size_t n = 0 ; n < count ; ++n)
	{
		GameState* state = new GameState();
		loadState(i, *state);
		game.push_back(state);
		deleteList.push_back(state);
	}
}
void Episode::save(ofstream& o)
{
	//o << role << endl;
	o << returns.size();
	for(size_t n = 0 ; n < returns.size() ; ++n)
	{
		o << " " << returns[n];
	}
	o << endl;
	//o << reward << endl;
	o << game.size() << endl;
	for(size_t n = 0 ; n < game.size() ; ++n)
	{
		saveState(o, *(game[n]));
		o << StateDelimiter;
	}
}

void Episode::loadState(ifstream& i, GameState& state)
{
	size_t count = 0;
	i >> count;
	size_t index;
	string line = "";
	char buffer[1024];
	for(size_t n = 0 ; n < count ; ++n)
	{
		//i.getline(buffer, 1024);
		i >> buffer;
		line = buffer;
		index = 0;
		state.push_back(Compound::fromSaveString(line, index));
	}
	i >> buffer; //delimiter
}
void Episode::saveState(ofstream& o, GameState& state)
{
	o << state.size() << endl;
	string line = "";
	for(size_t n = 0 ; n < state.size() ; ++n)
	{
		line = Compound::toSaveString(state[n]);
		line += "\n";
		o << line.c_str();
	}
}

void Episode::generate(GameTheory& theory)
{
	theory.markGameState();
	this->addState(theory.getStateRef());
	PlayMoves pm;
	RandomPlayer player;
	player.newGame(theory);
	while(!theory.isTerminal())
	{
		for(size_t n = 0 ; n < theory.getRoles()->size() ; ++n)
		{
			player.setRole(n, theory.getRole(n));
			pm.push_back(player.play(theory));
			//cout << player.getPlayerName().c_str() << " as " << player.role.c_str() 
			//	<< " plays " << move.compound.toString(theory.getSymbols()).c_str() << endl;
		}
		cout << ".";
		flush(cout);
		theory.make(pm);
		this->addState(theory.getStateRef());
		pm.clear();
	}
	cout << endl;
	returns.resize(theory.getRoles()->size(), GOAL_NOT);
	for(size_t n = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		setReturn(n, theory.goal(n));
	}
	theory.retractToMark();
}
double Episode::generate(GameTheory& theory, GamePlayers& players, RoleIndex roleIndex)
{
	//this->setRole((*(theory.getRoles()))[roleIndex]);
	theory.markGameState();
	this->addState(theory.getStateRef());
	PlayMoves pm;
	while(!theory.isTerminal())
	{
		for(size_t n = 0 ; n < players.size() ; ++n)
		{
			players[n]->newGame(theory);
			pm.push_back(players[n]->play(theory));
			//cout << players[n]->getPlayerName().c_str() << " as " << players[n]->role.c_str() 
			//	<< " plays " << move.compound.toString(theory.getSymbols()).c_str() << endl;
		}
		theory.make(pm);
		this->addState(theory.getStateRef());
		pm.clear();
	}
	returns.resize(theory.getRoles()->size(), GOAL_NOT);
	for(size_t n = 0 ; n < theory.getRoles()->size() ; ++n)
	{
		setReturn(n, theory.goal(n));
	}
	theory.retractToMark();
	return returns[roleIndex];
}
string Episode::toString(GameTheory* theory)
{
	SymbolTable* symbols = theory->getSymbols();
	string str = "";
	char buffer[64];
	for(size_t n = 0 ; n < game.size() ; ++n)
	{
		sprintf(buffer, "State %u.\n", (unsigned int)(n+1));
		str += buffer;
		str += GameTheory::listToString(game[n], symbols);
	}
	Role role; 
	for(size_t n = 0 ; n < theory->getRoles()->size() ; ++n)
	{
		role = (*(theory->getRoles()))[n];
		str += "\nReward as the role of ";
		str += role;
		sprintf(buffer, " : %f\n", returns[n]);
		str += buffer;
	}
	return str;	
}

//-------------------------
//  Episodes class
//-------------------------

Episodes::~Episodes()
{
	this->clear();
}

void Episodes::transferEpisodesTo(Episodes& to)
{
	for(size_t n = 0 ; n < eps.size() ; ++n)
	{
		to.push_back(eps[n]);
	}
	eps.clear();
}

void Episodes::clear()
{
	for(size_t n = 0 ; n < eps.size() ; ++n)
	{
		if(eps[n] != NULL)
		{
			try{delete eps[n];}
			catch(...){}
		}
	}
	eps.clear();
}

void Episodes::generate(GameTheory& theory, size_t count)
{
	for(size_t n = 0 ; n < count ; ++n)
	{
#ifdef DEBUG_TRAINER
		cout << "Iteration " << (n+1) << endl;
#endif
		Episode* episode = new Episode();
		episode->generate(theory);
		eps.push_back(	episode );
	}
}
void Episodes::generate(GameTheory& theory, GamePlayers& players, RoleIndex roleIndex, size_t count)
{
	//double sumReward = 0;
	for(size_t n = 0 ; n < count ; ++n)
	{
#ifdef DEBUG_TRAINER
		cout << "Iteration " << (n+1) << endl;
#endif
		Episode* episode = new Episode();
		//sumReward += episode->generate(theory, players, roleIndex);
		episode->generate(theory, players, roleIndex);
		eps.push_back(	episode );
	}
	//return sumReward;
}
void Episodes::load(string file)
{
	ifstream i;
	i.open(file.c_str());
	size_t count = 0;
	char delim[3];
	i >> count;
	for(size_t n = 0 ; n < count ; ++n)
	{
		Episode* episode = new Episode();
		episode->load(i);
		eps.push_back(episode);
		i >> delim;
	}
	i.close();
}
void Episodes::save(string file)
{
	ofstream o;
	o.open(file.c_str());
	o << eps.size() << endl;
	for(size_t n = 0 ; n < eps.size() ; ++n)
	{
		eps[n]->save(o);
		o << EpisodeDelimiter;
	}
	o.flush();
	o.close();
}


string Episodes::toString(GameTheory* theory)
{
	string str = "";
	char buffer[64];
	for(size_t n = 0 ; n < eps.size() ; ++n)
	{
		sprintf(buffer, "Episode %u.\n", (unsigned int)(n+1));
		str += buffer;
		str += eps[n]->toString(theory);
		str += "\n";
	}
	return str;
}
