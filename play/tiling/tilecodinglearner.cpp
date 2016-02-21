/*
 *  tilecodinglearner.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "tilecodinglearner.h"

using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::heuristic;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::tiling;

TileCodingLearner::TileCodingLearner(cadiaplayer::play::GameTheory* t, cadiaplayer::play::RoleIndex r):
theory(t),
role(r),
externalTiles(false)
{
	tiles = new TileCoding(t->getSymbols());
}
TileCodingLearner::TileCodingLearner(cadiaplayer::play::GameTheory* t, cadiaplayer::play::RoleIndex r, TileCoding* tc):
theory(t),
role(r),
externalTiles(true),
tiles(tc)
{}

TileCodingLearner::~TileCodingLearner(void)
{
	if(!externalTiles)delete tiles;
}
TileCoding* TileCodingLearner::getTiles(void)
{
	return tiles;
}


void TileCodingLearner::initialize(void)
{
	//TileArguments args;
	//CompoundTiling* t1 = new CompoundTiling(theory->getSymbols(), args);
	//tiles->addTiling((Tiling*)t1);
	
	//TileArgument arg = 0;
	/*args.push_back(arg);
	 Tiling* t2 = new CompoundValueTiling(theory->getSymbols(), args);
	 tiles->addTiling(t2);
	 args.clear();
	 arg = 1;
	 args.push_back(arg);
	 Tiling* t3 = new CompoundValueTiling(theory->getSymbols(), args);
	 tiles->addTiling(t3);
	 args.clear();*/
	//arg = 2;
	//args.push_back(arg);
	//Tiling* t4 = new CompoundValueTiling(theory->getSymbols(), args);
	//tiles->addTiling(t4);
	
	//ValueTiling* t = new ValueTiling(theory->getSymbols());
	//tiles->addTiling((Tiling*)t);
}

void TileCodingLearner::simulateEpisode(void)
{
	Episode episode;
	theory->markGameState();
	episode.addState(theory->getStateRef());
	bool terminal = theory->isTerminal();
	while(!terminal)
	{
		theory->playRandomMove();
		episode.addState(theory->getStateRef());
		
		terminal = theory->isTerminal();
	}
	for(size_t n = 0 ; n < theory->getRoles()->size() ; n++)
	{
		episode.setReturn(n, theory->goal(n));
	}
	playEpisode(episode);
	theory->retractToMark();
}

void TileCodingLearner::playEpisode(Episode& episode)
{
	if(episode.size() < 2)
		return;
	
	double v_s = 0.0;
	double v_s_prime = GOAL_NOT;
	double delta_s;
	GameState* s = episode[0];
	GameState* s_prime;
	size_t terminal = episode.size()-1;
	size_t rPos = episode.size()-2;
	double r = 0;
	std::vector<double>& returns = episode.getReturns();
	for(size_t n = 0 ; n < returns.size() ; n++)
	{
		if(n==role)
			r += returns[n];
		else
			r -= returns[n];
	}
	FeatureVectors fvList;
	fvList.resize(terminal);
	size_t maxFeatures = tiles->getFeaturesPresent(s, fvList[0]);
	size_t countFeatures;
	FeatureVector& f_s = fvList[0];
	Traces e;
	e.resize(terminal, 0);
	Weights w;
	for(size_t n = 0 ; n < weights.size() ; ++n)
	{
		w.push_back(weights[n]);
	}
	for(size_t n = 0 ; n < terminal ; ++n)
	{
		s = episode[n];
		s_prime = episode[n+1];
		
		f_s = fvList[n];
		v_s = getValue(f_s);
		if(n < rPos)
		{
			countFeatures = tiles->getFeaturesPresent(s_prime, fvList[n+1]);
			v_s_prime = getValue(fvList[n+1]);
			if(maxFeatures < countFeatures)
				maxFeatures = countFeatures;
		}	
		else
			v_s_prime = r;
		v_s_prime*=TILECODINGLEARNER_GAMMA;
		
		//std::cerr << "delta = v_s_prime-v_s -> " << delta << " = " << v_s_prime << " - " << v_s << std::endl;
		delta_s = v_s_prime-v_s;
		delta_s/=maxFeatures;
		e[n] = 1; 
		if(w.size() < weights.size())
			w.resize(weights.size(), TILECODINGLEARNER_INIT_WEIGHT);
		for(size_t m = 0 ; m <= n ; m++)
		{
			updateWeights(delta_s, w, fvList[m], e[m]);
			
			e[m] = TILECODINGLEARNER_GAMMA*TILECODINGLEARNER_LAMBDA * e[m];
			if(ZEROMARGIN(e[m]))
				e[m] = 0;
		}
		
	}
	setWeights(w);
	lastEpisodeGoal = r;
	//std::cerr << "lastEpisodeGoal : " << lastEpisodeGoal << std::endl; 
}

std::size_t TileCodingLearner::playEpisode(Episode& episode, double absweight, std::size_t samples)
{
	if(episode.size()< 2)
		return false;
	
	size_t valid = 0;
	size_t maxvalid = 0;
	double v_s = 0.0;
	double v_s_prime = GOAL_NOT;
	double delta_s;
	GameState* s = episode[0];
	GameState* s_prime;
	size_t terminal = episode.size()-1;
	size_t rPos = episode.size()-2;
	double r = 0;
	std::vector<double>& returns = episode.getReturns();
	for(size_t n = 0 ; n < returns.size() ; n++)
	{
		if(n==role)
			r += returns[n];
		else
			r -= returns[n];
	}
	FeatureVectors fvList;
	fvList.resize(terminal);
	size_t maxFeatures = tiles->getFeaturesPresent(s, fvList[0]);
	size_t countFeatures;
	FeatureVector& f_s = fvList[0];
	Traces e;
	e.resize(terminal, 0);
	Weights w;
	for(size_t n = 0 ; n < weights.size() ; ++n)
	{
		w.push_back(weights[n]);
	}
	for(size_t n = 0 ; n < terminal ; ++n)
	{
		s = episode[n];
		s_prime = episode[n+1];
		
		f_s = fvList[n];
		v_s = getValue(f_s);
		if(n < rPos)
		{
			countFeatures = tiles->getFeaturesPresent(s_prime, fvList[n+1]);
			v_s_prime = getValue(fvList[n+1]);
			if(maxFeatures < countFeatures)
				maxFeatures = countFeatures;
		}	
		else
			v_s_prime = r;
		v_s_prime*=TILECODINGLEARNER_GAMMA;
		
		delta_s = v_s_prime-v_s;
		//std::cerr << "delta = v_s_prime-v_s -> " << delta << " = " << v_s_prime << " - " << v_s << std::endl;
		delta_s/=maxFeatures;
		e[n] = 1; 
		if(w.size() < weights.size())
			w.resize(weights.size(), TILECODINGLEARNER_INIT_WEIGHT);
		for(size_t m = 0 ; m <= n ; m++)
		{
			valid = updateWeights(delta_s, w, fvList[m], e[m], absweight, samples);
			if(valid > maxvalid)
				maxvalid = valid;
			
			e[m] = TILECODINGLEARNER_GAMMA*TILECODINGLEARNER_LAMBDA * e[m];
			if(ZEROMARGIN(e[m]))
				e[m] = 0;
		}
		
	}
	setWeights(w);
	lastEpisodeGoal = r;
	//std::cerr << "lastEpisodeGoal : " << lastEpisodeGoal << std::endl; 
	return maxvalid;
}


void TileCodingLearner::updateWeights(const double delta, Weights& w, const FeatureVector& fv, const Trace t)
{
	if(t == 0)
		return;
	for(size_t n = 0 ; n < fv.size() ; ++n)
	{
		if(!fv[n])
			continue;
		w[n] = weights[n] + TILECODINGLEARNER_ALPHA*delta*t*fv[n];
		//std::cerr << w[n] << " = " << weights[n] << " + " << TILECODINGLEARNER_ALPHA << "*" << delta << "*" << t << "*" << fv[n] << std::endl;
#ifndef TILECODINGLEARNER_COUNTERS_OFF
		if(delta > 0)
			localchange[n]++;
		else if( delta < 0)
			localchange[n]--;
#endif
		frequencycount[n]++;
	}
}
size_t TileCodingLearner::updateWeights(const double delta, Weights& w, const FeatureVector& fv, const Trace t, double absweight, std::size_t samples)
{
	if(t == 0)
		return false;
	size_t valid = 0;
	for(size_t n = 0 ; n < fv.size() ; ++n)
	{
		if(!fv[n])
			continue;
		w[n] = weights[n] + TILECODINGLEARNER_ALPHA*delta*t*fv[n];
		//std::cerr << w[n] << " = " << weights[n] << " + " << TILECODINGLEARNER_ALPHA << "*" << delta << "*" << t << "*" << fv[n] << std::endl;
#ifndef TILECODINGLEARNER_COUNTERS_OFF
		if(delta > 0)
			localchange[n]++;
		else if( delta < 0)
			localchange[n]--;
#endif
		frequencycount[n]++;
		
		if(frequencycount[n] >= samples && (w[n] > absweight || -w[n] > absweight))
		   ++valid;
	}
	return valid;
}
double TileCodingLearner::getValue(GameState* state)
{
	FeatureVector features = tiles->getFeaturesPresent(state);
	double stateValue = 0;
	for(size_t n = 0 ; n < features.size() ; ++n)
	{
		if(weights.size() <= n)
			break;
		stateValue += weights[n] * features[n];
	}
	return stateValue;
}
double TileCodingLearner::getValue(const FeatureVector& features)
{
	if(weights.size() < features.size())
		resizeWeights(features.size());
	double stateValue = 0;
	for(size_t n = 0 ; n < features.size() ; ++n)
	{
		stateValue += weights[n]*features[n];
	}
	return stateValue;
}

void TileCodingLearner::resizeWeights(size_t newsize)
{
#ifdef DEBUG_TRAINER
	cout << "Resizing weights to " << newsize << endl;
#endif
	weights.resize(newsize, TILECODINGLEARNER_INIT_WEIGHT);
#ifndef TILECODINGLEARNER_COUNTERS_OFF
	localchange.resize(newsize, 0);
#endif
	frequencycount.resize(newsize, 0);
}

double TileCodingLearner::evaluate(GameTheory* theory, RoleIndex r)
{
	if(r == role)
		return getValue(theory->getStateRef());
	return -getValue(theory->getStateRef());
}

double TileCodingLearner::getWeight(TileKey feature)
{
	FeatureIndex index = tiles->getIndex(feature);
	if(weights.size() <= index)
		return 0;
	double score = weights[index];
	return score;
}

string TileCodingLearner::getInfo(void)
{
	string str = getWeightInfo();
	str += tiles->getInfo();
	return str;
}
string TileCodingLearner::getQuickInfo(void)
{
	stringstream ss;
	ss << "Weights count : " << weights.size() << std::endl;
	ss << tiles->getQuickInfo() << std::endl;
	return ss.str();
}

string TileCodingLearner::getWeightInfo(void)
{
	stringstream str;
	str << "Episode reward : " << lastEpisodeGoal << "\n";
	str << "---------------------------------------";
#ifndef TILECODINGLEARNER_COUNTERS_OFF
	str << "---------------------";
#endif
	str << "\n             ";
#ifndef TILECODINGLEARNER_COUNTERS_OFF
	str << "                     ";
#endif
	str	<< "Weights    Feature\n";
	for(size_t n = 0 ; n < weights.size() ; ++n)
	{
		str.width(4);
		str << n;
#ifndef TILECODINGLEARNER_COUNTERS_OFF
		str.width(10);
		str << localchange[n];
		str.width(1);
		str << "/";
		str.width(10);
		str << frequencycount[n];
#endif
		str.width(16);
		str << weights[n];
		str << " -> " << tiles->getFeatureInfo(n) << "\n";
	}
	return str.str();
}

string TileCodingLearner::getTraceInfo(const Traces& e)
{
	char buffer[32];
	string str = "------------------\n      Traces\n------------------\n";
	for(size_t n = 0 ; n < e.size() ; ++n)
	{
		sprintf(buffer, "%u\t%f\n", (unsigned int)n, e[n]);
		str += buffer;
	}
	return str;	
}

