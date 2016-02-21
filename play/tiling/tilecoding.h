/*
 *  tilecoding.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef TILECODING_H
#define TILECODING_H

#include "tiling.h"
#include "../parsing/compound.h"
#include "../heuristic/heuristic.h"
#include "../utils/episode.h"
#include <ext/hash_map>
#include <vector>
#include <algorithm>

#define USE_TILE_DESCRIPTIONS
//#define DEBUG_TILES
 
#ifdef DEBUG_TILES
#include <iostream>
#endif

#define FEATUREINDEX_NOT_FOUND LONG_MAX

namespace cadiaplayer {
	namespace play { 
		namespace tiling {
			
			typedef unsigned int FeatureIndex;
			typedef unsigned int FeatureCounter;
			
			typedef __gnu_cxx::hash_map<TileKey, FeatureIndex> FeatureMap;
			typedef std::vector<TileKey> FeatureOrderedMap;
#ifdef USE_TILE_DESCRIPTIONS
			typedef std::vector<std::string> FeatureDescriptions;
#endif
			typedef std::vector<FeatureCounter> FeatureVector;
			typedef std::vector<FeatureVector> FeatureVectors;
			typedef std::vector<Tiling*> TilingsList;
			
			typedef double Stability;
			typedef std::vector<Stability> StabilityVector;
			
			class TileCoding
				{
				private:
					FeatureCounter counter;
					cadiaplayer::play::parsing::SymbolTable* symbols;
					TilingsList tilings;
					FeatureMap features;
					FeatureOrderedMap orderedFeatures;
					std::size_t minParameters;
#ifdef USE_TILE_DESCRIPTIONS
					FeatureDescriptions descriptions;
#endif
				public:
					TileCoding(cadiaplayer::play::parsing::SymbolTable* s);
					~TileCoding();
					
					void addTiling(Tiling* t){tilings.push_back(t);};
					void addKey(TileKey key);
					void addKey(TileKey key, std::string description);
					std::size_t getMinParameters(){return minParameters;};
					void setMinParameters(std::size_t mp){minParameters=mp;};
					FeatureVector getFeaturesPresent(cadiaplayer::play::parsing::Compound* c);
					std::size_t getFeaturesPresent(cadiaplayer::play::parsing::Compound* c, FeatureVector& f);
					FeatureVector getFeaturesPresent(cadiaplayer::play::parsing::CompoundList* cList);
					std::size_t getFeaturesPresent(cadiaplayer::play::parsing::CompoundList* cList, FeatureVector& f);
					TilingsList& getTilings(void){return tilings;};
					FeatureCounter getFeatureCount(void){return counter;};
					TileKey getFeature(FeatureIndex index){return orderedFeatures[index];};
					TileKey getFeature(cadiaplayer::play::parsing::Compound* c)
					{
						TileKey key = 0;
						for(size_t n = 0 ; n < tilings.size() ; n++)
						{
							key = tilings[n]->getFeature(c);
							if(key)
								break;
						}
						return key;
					};
					FeatureIndex getIndex(TileKey key)
					{
						FeatureMap::iterator f = features.find(key);
						if(f==features.end())
							return (unsigned int)FEATUREINDEX_NOT_FOUND;
						return f->second;
					}
#ifdef USE_TILE_DESCRIPTIONS
					std::string getFeatureDescription(FeatureIndex i);
#endif
					StabilityVector getCardinalityStability(cadiaplayer::play::utils::Episode& episode);
					StabilityVector getCardinalityStability(cadiaplayer::play::utils::Episodes& episodes);
					
					//double evalState(CompoundList* cList, Weights& weights);
					//double evalGoal(double reward);
					
					std::string getInfo(void);
					std::string getInfo(Tiling* tiling);
					std::string getQuickInfo(void);
					std::string getFeatureInfo(int index);
				private:
					FeatureIndex getIndex(TileKey key
#ifdef USE_TILE_DESCRIPTIONS
										  , Tiling* t, cadiaplayer::play::parsing::Compound* c
#endif
										  );
				};
		}}}			
#endif // TILECODING_H
