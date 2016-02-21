/*
 *  tilecodinglearner.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef TILECODINGLEARNER_H
#define TILECODINGLEARNER_H

#include "tilecoding.h"
#include "tilings.h"
#include "../gametheory.h"
#include "../heuristic/heuristic.h"
#include "../utils/episode.h"
#include <vector>
#include <cmath>
#include <sstream>

#define TILECODINGLEARNER_COUNTERS_OFF	// Do not use counter on win/loose delta per feature

#define SIGMOID(x) 1/(1+exp(-x))

#define 	TILECODINGLEARNER_GAMMA			0.999	// Close to no decay
#define 	TILECODINGLEARNER_LAMBDA		0.99	// Fade traces by 5%
#define 	TILECODINGLEARNER_ALPHA			0.01	// Step size of 1%
#define 	TILECODINGLEARNER_ZERO_MARGIN	1e-6	// Margin for zero in traces and weights.
#define		TILECODINGLEARNER_INIT_WEIGHT	0.0

#define 	ZEROMARGIN(a) (a < TILECODINGLEARNER_ZERO_MARGIN && a > -TILECODINGLEARNER_ZERO_MARGIN) ? true : false

namespace cadiaplayer {
	namespace play {
		namespace tiling {
#ifndef TILECODINGLEARNER_COUNTERS_OFF
			typedef std::vector<int> LocalChange;
#endif
			typedef std::vector<int> FrequencyCount;
			typedef double				Trace;
			typedef std::vector<Trace> Traces;
			
			class TileCodingLearner : public cadiaplayer::play::heuristic::Heuristic
				{
				private:
					cadiaplayer::play::GameTheory* theory;
					cadiaplayer::play::RoleIndex role;
					TileCoding* tiles;
#ifndef TILECODINGLEARNER_COUNTERS_OFF					
					double delta;
					LocalChange localchange;
#endif
					FrequencyCount frequencycount;
					std::vector<std::string> info;
					bool externalTiles;
					double lastEpisodeGoal;
					
				public:
					TileCodingLearner(cadiaplayer::play::GameTheory* t, cadiaplayer::play::RoleIndex r);
					TileCodingLearner(cadiaplayer::play::GameTheory* t, cadiaplayer::play::RoleIndex r, TileCoding* tc);
					~TileCodingLearner(void);
					TileCoding* getTiles(void);
					virtual void initialize(void);
					void simulateEpisode(void);
					void playEpisode(cadiaplayer::play::utils::Episode& episode);
					std::size_t playEpisode(cadiaplayer::play::utils::Episode& episode, double absweight, std::size_t samples);
					std::size_t getMinParameters(){return tiles? tiles->getMinParameters() : 0;};
					void setMinParameters(std::size_t mp){if(tiles)tiles->setMinParameters(mp);};
					
					// Heuristic inherited
					double evaluate(cadiaplayer::play::GameTheory* theory, cadiaplayer::play::RoleIndex r);
					double getWeight(TileKey feature);
					void setWeights(cadiaplayer::play::heuristic::Weights w)
					{weights.clear();weights.insert(weights.end(), w.begin(), w.end());};
					
					std::string getInfo(void);
					std::string getQuickInfo(void);
					std::string getWeightInfo(void);
					std::string getTraceInfo(const Traces& e);
				private:
					Move* selectAction(void);
					double getValue(cadiaplayer::play::GameState* state);
					double getValue(const FeatureVector& features);
					void updateWeights(const double delta, cadiaplayer::play::heuristic::Weights& w, const FeatureVector& fv, const Trace t);
					std::size_t updateWeights(const double delta, cadiaplayer::play::heuristic::Weights& w, const FeatureVector& fv, const Trace t, double absweight, std::size_t samples);
					void resizeWeights(std::size_t newsize);
				};
		}}}			
#endif // TILECODINGLEARNER_H
