/*
 *  episode.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef EPISODE_H
#define EPISODE_H

#include "../gametheory.h"
#include "../players/randomplayer.h"
#include "../parsing/symtab.h"
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

namespace cadiaplayer{
	namespace play{
		namespace utils{
			
			static std::string DefaultEpisodeFilename = "episode.txt";
			static std::string DefaultEpisodeCollectionFilename = "episodes.txt";
			static std::string StateDelimiter = "+\n";
			static std::string EpisodeDelimiter = "#\n";
			
			typedef std::vector<GameState*> EpisodeStates;
			
			class Episode
			{
			private:
				std::string filename;
				EpisodeStates game;
				EpisodeStates deleteList;
				std::vector<double> returns;
			public:
				Episode(){filename = DefaultEpisodeFilename;/*reward=GOAL_NOT;*/};
				~Episode(){this->clear();};
				
				GameState* operator[](const size_t& index){assert(index<game.size());return game[index];};
				
				std::string getFilename(){return filename;};
				void setFilename(std::string file){filename = file;};
				double getReturn(cadiaplayer::play::RoleIndex roleindex){return returns[roleindex];};
				std::vector<double>& getReturns(void){return returns;};
				void setReturn(cadiaplayer::play::RoleIndex roleindex, double r){returns[roleindex] = r;};
				void setReturns(std::vector<double> r)
				{
					if(returns.size() < r.size())
						returns.resize(r.size());
					for(size_t n = 0 ; n < r.size() ; n++)
					{
						returns[n] = r[n];
					}
				};
				void addState(cadiaplayer::play::GameState* state)
				{
					/*cadiaplayer::play::GameState s;
					 for(size_t n = 0 ; n < state->size() ; ++n)
					 {
					 cadiaplayer::play::parsing::Compound* c = new cadiaplayer::play::parsing::Compound(*((*state)[n]));
					 s.push_back(c);
					 }
					 game.push_back(s);*/
					game.push_back(state);
				};
				
				void load(){load(filename);};
				void save(){save(filename);};
				void load(std::string file);
				void save(std::string file);
				void load(std::ifstream& i);
				void save(std::ofstream& o);
				
				size_t size(){return game.size();};
				void clear();
				std::string toString(cadiaplayer::play::GameTheory* theory);
				
				void generate(cadiaplayer::play::GameTheory& theory);
				// returns the reward from the episode
				double generate(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::players::GamePlayers& players, cadiaplayer::play::RoleIndex roleIndex);
				
			private:
				void loadState(std::ifstream& i, cadiaplayer::play::GameState& state);
				void saveState(std::ofstream& o, cadiaplayer::play::GameState& state);
			};
			
			class Episodes
			{
			private:
				std::vector<Episode*> eps;
			public:
				~Episodes();
				
				Episode* operator[](const size_t& index){assert(index<eps.size());return eps[index];};
				void push_back(Episode* e){eps.push_back(e);};
				size_t size(){return eps.size();};
				void clear();
				void transferEpisodesTo(Episodes& from);
				
			public:	
				void generate(cadiaplayer::play::GameTheory& theory, size_t count);
				void generate(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::players::GamePlayers& players, cadiaplayer::play::RoleIndex roleIndex, size_t count);
				void load(std::string file = DefaultEpisodeCollectionFilename);
				void save(std::string file = DefaultEpisodeCollectionFilename);
				std::string toString(cadiaplayer::play::GameTheory* theory);
			};
		}}}

#endif // EPISODE_H
