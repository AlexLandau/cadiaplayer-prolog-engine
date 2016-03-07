/*
 *  cadiaplayer.cpp
 *  cadiaplayer
 *  main program w/o server
 *
 *  Created by Hilmar Finnsson on 9/12/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 *  Modifications by Alex Landau to function as a performance
 *  test agent.
 */

#include "agent.h"
#include "../play/players/randomplayer.h"
#include "../play/players/tomastplayer.h"
#include "../play/players/pastplayer.h"
#include "../play/players/tomastraveplayer.h"
#include "../play/players/pastraveplayer.h"
#include "../play/players/fastplayer.h"
#include "../play/players/fastraveplayer.h"
#include "../play/players/fastmastplayer.h"
#include "../play/players/fastmastraveplayer.h"
#include "../play/players/fasttomastplayer.h"
#include "../play/players/fasttomastraveplayer.h"
#include "../play/players/fastpastplayer.h"
#include "../play/players/fastpastraveplayer.h"

#include "../utils/settings.h"
// Settings constants
#define VERSION "3.0"
#define SETTINGS_FILE			"cadiaplayer.ini"
#define SETTING_INT_DISABLED	-1
#define SET_PLAYER_TYPE			"TYPE"
#define SET_PLAYER_SELECTION	"PLAYER"
#define SET_PLAYER_SELECTION_RANDOM	0
#define SET_PLAYER_SELECTION_UCT	1
#define SET_PLAYER_SELECTION_MAST	2
#define SET_PLAYER_SELECTION_TOMAST	3
#define SET_PLAYER_SELECTION_PAST	4
#define SET_UCT_C				"UCT"
#define SET_MAST_TAU			"MAST"
#define SET_PAST_TAU			"PAST"
#define SET_FAST				"FAST"
#define SET_RAVE_K				"RAVE"
#define SET_UAU_SCORE			"UAU"
#define SET_EC_MIN				"EC"
#define SET_SIMULATION_LIMIT	"SIMLIM"
#define SET_EXPANSIONS_LIMIT	"EXPLIM"
#define SET_LOGFILE				"LOGFILE"
#define SET_EXTERNALS_FILE		"EXTFILE"


using namespace cadiaplayer::agent;
using namespace cadiaplayer::utils;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::players;

// Early cutoff
void setEarlyCutoff(Settings& settings, UCTPlayer* player)
{
	int ec = SETTING_INT_DISABLED;
	if(settings.getIntValue(SET_EC_MIN, ec))
	{
		if(ec != SETTING_INT_DISABLED)
			player->enableEarlyCutoff(ec);
		else
			player->disableEarlyCutoff();
	}
	else
		player->disableEarlyCutoff();
}

// Unexplored action urgency
void setUnexploredActionUrgency(Settings& settings, UCTPlayer* player)
{
	double uau = SETTING_INT_DISABLED;
	if(settings.getDoubleValue(SET_UAU_SCORE, uau))
	{
		if(uau != SETTING_INT_DISABLED)
			player->enableUnexploredActionUrgency(uau);
		else
			player->disableUnexploredActionUrgency();
	}
	else
		player->disableUnexploredActionUrgency();
}

void limitSimulations(Settings& settings, UCTPlayer* player)
{
	int val;
	if(settings.getIntValue(SET_SIMULATION_LIMIT, val))
	{
		if(val > 0)
			player->limitSimulations(val);
	}
}
void limitNodeExpansions(Settings& settings, UCTPlayer* player)
{
	int val;
	if(settings.getIntValue(SET_EXPANSIONS_LIMIT, val))
	{
		if(val > 0)
			player->limitNodeExpansions(val);
	}
}


UCTPlayer* getUCTPlayer(Settings& settings)
{
	UCTPlayer* player = NULL;
	int fast = 0;
	settings.getIntValue(SET_FAST, fast);
	int k;
	bool rave = settings.getIntValue(SET_RAVE_K, k);
	if(rave && k == SETTING_INT_DISABLED)
		rave = false;
	
	if(rave)
	{
		if(fast)
			player = new FASTRAVEPlayer();
		else
			player = new UCTRAVEPlayer();
		static_cast<UCTRAVEPlayer*>(player)->setEquivalenceParameter(k);
	}
	else 
	{
		if(fast)
			player = new FASTPlayer();
		else
			player = new UCTPlayer();
	}
	
	double c;
	if(settings.getDoubleValue(SET_UCT_C, c))
		player->setCValues(c);
	
	return player;
}
UCTPlayer* getMASTPlayer(Settings& settings)
{
	UCTPlayer* player = NULL;
	int fast = 0;
	settings.getIntValue(SET_FAST, fast);
	int k;
	bool rave = settings.getIntValue(SET_RAVE_K, k);
	if(rave && k == SETTING_INT_DISABLED)
		rave = false;
	
	if(rave)
	{
		if(fast)
			player = new FASTMASTRAVEPlayer();
		else
			player = new MASTRAVEPlayer();
		static_cast<UCTRAVEPlayer*>(player)->setEquivalenceParameter(k);
		double tau;
		if(settings.getDoubleValue(SET_MAST_TAU, tau))
			static_cast<UCTRAVERolloutPlayer*>(player)->setTemperature(tau);
	}
	else 
	{
		if(fast)
			player = new FASTMASTPlayer();
		else
			player = new MASTPlayer();
		double tau;
		if(settings.getDoubleValue(SET_MAST_TAU, tau))
			static_cast<UCTRolloutPlayer*>(player)->setTemperature(tau);
	}
	
	double c;
	if(settings.getDoubleValue(SET_UCT_C, c))
		player->setCValues(c);
	return player;
}
UCTPlayer* getTOMASTPlayer(Settings& settings)
{
	UCTPlayer* player = NULL;
	int fast = 0;
	settings.getIntValue(SET_FAST, fast);
	int k;
	bool rave = settings.getIntValue(SET_RAVE_K, k);
	if(rave && k == SETTING_INT_DISABLED)
		rave = false;
	
	if(rave)
	{
		if(fast)
			player = new FASTTOMASTRAVEPlayer();
		else
			player = new TOMASTRAVEPlayer();
		static_cast<UCTRAVEPlayer*>(player)->setEquivalenceParameter(k);
		double tau;
		if(settings.getDoubleValue(SET_MAST_TAU, tau))
			static_cast<UCTRAVERolloutPlayer*>(player)->setTemperature(tau);
	}
	else 
	{
		if(fast)
			player = new FASTTOMASTPlayer();
		else
			player = new TOMASTPlayer();
		double tau;
		if(settings.getDoubleValue(SET_MAST_TAU, tau))
			static_cast<UCTRolloutPlayer*>(player)->setTemperature(tau);
	}
	
	double c;
	if(settings.getDoubleValue(SET_UCT_C, c))
		player->setCValues(c);
	return player;
}
UCTPlayer* getPASTPlayer(Settings& settings)
{
	UCTPlayer* player = NULL;
	int fast = 0;
	settings.getIntValue(SET_FAST, fast);
	int k;
	bool rave = settings.getIntValue(SET_RAVE_K, k);
	if(rave && k == SETTING_INT_DISABLED)
		rave = false;
	
	if(rave)
	{
		if(fast)
			player = new FASTPASTRAVEPlayer();
		else
			player = new PASTRAVEPlayer();
		static_cast<UCTRAVEPlayer*>(player)->setEquivalenceParameter(k);
		double tau;
		if(settings.getDoubleValue(SET_PAST_TAU, tau))
			static_cast<UCTRAVERolloutPlayer*>(player)->setTemperature(tau);
	}
	else 
	{
		if(fast)
			player = new FASTPASTPlayer();
		else
			player = new PASTPlayer();
		double tau;
		if(settings.getDoubleValue(SET_PAST_TAU, tau))
			static_cast<UCTRolloutPlayer*>(player)->setTemperature(tau);
	}
	
	double c;
	if(settings.getDoubleValue(SET_UCT_C, c))
		player->setCValues(c);
	return player;
}

GamePlayer* getPlayer(Settings& settings)
{
	UCTPlayer* player = NULL;
	int val;
	if(settings.getIntValue(SET_PLAYER_SELECTION, val))
	{
		switch (val) 
		{
			case SET_PLAYER_SELECTION_UCT:
				player = getUCTPlayer(settings);
				break;
			case SET_PLAYER_SELECTION_MAST:
				player = getMASTPlayer(settings);
				break;
			case SET_PLAYER_SELECTION_TOMAST:
				player = getTOMASTPlayer(settings);
				break;
			case SET_PLAYER_SELECTION_PAST:
				player = getPASTPlayer(settings);
				break;
			default:
				return new RandomPlayer();
				break;
		}
	}
	else // Default handling if the setting is missing
		return new RandomPlayer();
	
	// Early cutoff
	setEarlyCutoff(settings, player);
	
	// Unexplored action urgency
	setUnexploredActionUrgency(settings, player);
	
	limitSimulations(settings, player);
	limitNodeExpansions(settings, player);
	
	return player;
}

int stringToInt(char *str) {
  std::stringstream ss;
  ss << str;
  int i;
  ss >> i;
  return i;
}

//I've cannibalized this class so as to avoid having to mess with
//the Automake system in ways I don't understand.
int main(int argc, char *argv[])
{
  if (argc >= 2)
    std::cout << argv[1] << '\n';
  if (argc >= 3)
    std::cout << argv[2] << '\n';
  if (argc >= 4)
    std::cout << argv[3] << '\n';
  if (argc < 4)
  {
    std::cout << "Three arguments must be provided: game file, output file, and seconds to run\n";
    exit(1);
  }
  char* gameFilename = argv[1];
  char* outputFilename = argv[2];
  int secondsToRun = stringToInt(argv[3]);

  GameTheory theory;
  //delete theory;
  parsing::GameParser parser;
  parser.parseFile(&theory, gameFilename);
  std::cout << "a0\n";
  theory.setKifFile(gameFilename);
  std::cout << "a1\n";
  cadiaplayer::logic::PrologController controller;
  std::cout << "a2\n";
  theory.useController(&controller);
  std::cout << "a3\n";
  std::vector<int> goals;
  std::time_t startTime = std::time(NULL);
  int stateChangeCount = 0;
  int rolloutCount = 0;

  std::cout << "a\n";

  //Core loop
  while (std::time(NULL) - startTime < secondsToRun)
  {
    std::cout << "b\n";
    while (!theory.isTerminal())
    {
    std::cout << "c\n";
      theory.playRandomMove();
      stateChangeCount++;
    }
    theory.getGoalValues(goals);
    theory.retractAll();
    rolloutCount++;
  }
  std::time_t totalTime = std::time(NULL) - startTime;

  Settings output(outputFilename);
  output.set("version", VERSION);
  output.set("millisecondsTaken", ((int) totalTime) * 1000);
  output.set("numStateChanges", stateChangeCount);
  output.set("numRollouts", rolloutCount);
};
