/*
 *  tilecoding.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "tilecoding.h"
using namespace std;
using namespace cadiaplayer::play;
using namespace cadiaplayer::play::parsing;
using namespace cadiaplayer::play::utils;
using namespace cadiaplayer::play::tiling;

TileCoding::TileCoding(SymbolTable* s)
{
	minParameters = 2;
	counter = 0;
	symbols = s;
}
TileCoding::~TileCoding()
{
	for(size_t n = 0 ; n < tilings.size(); ++n)
	{
		delete tilings[n];
	}
	tilings.clear();
}

void TileCoding::addKey(TileKey key)
{
#ifdef DEBUG_TILES
	cerr << "Adding " << key << " to ordered list at pos " << counter << endl;
#endif
	orderedFeatures.push_back(key);
#ifdef DEBUG_TILES
	cerr << "Adding " << key << " to hash map" << endl;
#endif
	features[key] = counter++;
}
void TileCoding::addKey(TileKey key, string description)
{
#ifdef DEBUG_TILES
	cerr << "Adding " << key << " to ordered list at pos " << counter << endl;
#endif
	orderedFeatures.push_back(key);
#ifdef DEBUG_TILES
	cerr << "Adding " << key << " to hash map" << endl;
#endif
	features[key] = counter++;
#ifdef USE_TILE_DESCRIPTIONS
#ifdef DEBUG_TILES
	cerr << "Adding description for " << key << endl;
#endif
	descriptions.push_back(description);
#endif
}

FeatureVector TileCoding::getFeaturesPresent(Compound* c)
{
	FeatureVector presence;
	// Skip facts with none or one argument
	if(!c->getArguments() || c->getArguments()->size() < 2)
		return presence;
	
	TileKey key;
	FeatureIndex index;
	for(size_t n = 0 ; n < tilings.size() ; n++)
	{
#ifdef DEBUG_TILES
		cerr << "Looking up feature\n";
#endif
		key = tilings[n]->getFeature(c);
		if(key == FEATURE_NONE)
		{
#ifdef DEBUG_TILES
			cerr << "Feature not found\n";
#endif
			continue;
		}
		index = getIndex(key
#ifdef USE_TILE_DESCRIPTIONS
						 , tilings[n], c
#endif
						 );
		while(presence.size() < index+1)
		{
			presence.push_back(0);
		}
		++presence[index];
#ifdef DEBUG_TILES
		cerr << "Found feature\n";
#endif
	}
	return presence;
}
std::size_t TileCoding::getFeaturesPresent(Compound* c, FeatureVector& f)
{
	// Skip facts with none or one argument
	if(!c->getArguments() || c->getArguments()->size() < minParameters)
		return 0;
#ifdef DEBUG_TILES
	cerr << "Looking up feature for single compound\n";
#endif
	std::size_t count = 0;
	TileKey key;
	FeatureIndex index;
	for(size_t n = 0 ; n < tilings.size() ; n++)
	{
#ifdef DEBUG_TILES
		cerr << "Checking tiling #" << n << endl;
#endif
		key = tilings[n]->getFeature(c);
		if(key == FEATURE_NONE)
			continue;
#ifdef DEBUG_TILES
		cerr << "Feature found\n";
#endif
		index = getIndex(key
#ifdef USE_TILE_DESCRIPTIONS
						 , tilings[n], c
#endif
						 );
#ifdef DEBUG_TILES
		cerr << "Index found as " << index << " with feature vector size as " << f.size() << ", incrementing feature counter\n";
#endif
		while(f.size() < index+1)
		{ 
			f.push_back(0);
#ifdef DEBUG_TILES
			cerr << "Resizing f to " << index+1 << endl;
#endif
		}
		++f[index];
		++count;
#ifdef DEBUG_TILES
		cerr << "Feature counter incremented\n";
#endif
	}
	return count;
}
FeatureVector TileCoding::getFeaturesPresent(CompoundList* cList)
{
	FeatureVector presence;
	for(size_t n = 0 ; n < cList->size() ; n++)
	{
#ifdef DEBUG_TILES
		cerr << "Looking up features for " << (*cList)[n]->toString(symbols) << endl;
#endif
		getFeaturesPresent((*cList)[n], presence);
		//FeatureVector subpresence = getFeaturesPresent((*cList)[n]);
#ifdef DEBUG_TILES
		cerr << "Features returned\n";
#endif
		//presence.insert(presence.end(), subpresence.begin(), subpresence.end());
#ifdef DEBUG_TILES
		cerr << "Features merged\n";
#endif
	}
#ifdef DEBUG_TILES
	cerr << "Returning presence\n";
#endif
	return presence;
}
std::size_t TileCoding::getFeaturesPresent(cadiaplayer::play::parsing::CompoundList* cList, FeatureVector& f)
{
	std::size_t count = 0;
	for(std::size_t n = 0 ; n < cList->size() ; n++)
	{
#ifdef DEBUG_TILES
		cerr << "Looking up features for " << (*cList)[n]->toString(symbols) << endl;
#endif
		count += getFeaturesPresent((*cList)[n], f);
		//FeatureVector subpresence = getFeaturesPresent((*cList)[n]);
#ifdef DEBUG_TILES
		cerr << "Features returned\n";
#endif
		//presence.insert(presence.end(), subpresence.begin(), subpresence.end());
#ifdef DEBUG_TILES
		cerr << "Features merged\n";
#endif
	}
#ifdef DEBUG_TILES
	cerr << "Returning presence\n";
#endif
	return count;
}

FeatureIndex TileCoding::getIndex(TileKey key
#ifdef USE_TILE_DESCRIPTIONS
								  , Tiling* t, Compound* c
#endif
								  )
{
	FeatureMap::iterator fetch;
	fetch = features.find(key);
	if(fetch != features.end())
	{
#ifdef DEBUG_TILES
		cerr << "Found existing feature in map\n";
#endif
		return fetch->second;
	}
	
#ifdef DEBUG_TILES
	cerr << "Adding feature to map\n";
#endif
	addKey(key
#ifdef USE_TILE_DESCRIPTIONS
		   ,t->toString(c)
#endif
		   );
#ifdef DEBUG_TILES
	cerr << "Returning new index\n";
#endif
	return(counter-1);
}


StabilityVector TileCoding::getCardinalityStability(Episode& episode)
{
	StabilityVector sv;
	if(!episode.size())
		return sv;
	FeatureVector f = getFeaturesPresent(episode[0]);
	FeatureVector f_prime;
	
	FeatureVector max;
	max.insert(max.end(), f.begin(), f.end());
	FeatureVector min;
    min.insert(min.end(), f.begin(), f.end());
	
	for(size_t n = 1 ; n < episode.size() ; ++n)
	{
		if(sv.size() < f.size())
			sv.resize(f.size(), 0.0);
		f_prime = getFeaturesPresent(episode[n]);
		if(f_prime.size() < f.size())
			f_prime.resize(f.size(), 0);
		for(size_t i = 0 ; i < f.size() ; ++i)
		{
			if(f_prime[i] < min[i]) min[i] = f_prime[i];
			if(f_prime[i] > max[i]) max[i] = f_prime[i];
			sv[i] += pow(static_cast<double>(f_prime[i]-f[i]), 2.0);
		}
		f = f_prime;
	}
	if(episode.size() > 1)
	{
		FeatureCounter totalVariance = 0;
#ifdef USE_TILE_DESCRIPTIONS
		cerr << "Logged " << sv.size() << " features " << endl;
#endif
		for(size_t n = 0 ; n < sv.size() ; ++n)
		{
#ifdef USE_TILE_DESCRIPTIONS
			cerr << "Measuring feature " << n << endl;
#endif
			if(sv[n] == 0)
				continue;
			sv[n] /= (episode.size()-1);
#ifdef USE_TILE_DESCRIPTIONS
			cerr << "Adjacent variance of feature " << n << " measured as " << sv[n] << endl;
#endif
			totalVariance = max[n]-min[n];
#ifdef USE_TILE_DESCRIPTIONS
			cerr << "Total variance of feature " << n << " measured as " << totalVariance << endl;
#endif
			if(!totalVariance)
				continue;
			sv[n] /= totalVariance;
#ifdef USE_TILE_DESCRIPTIONS
			cerr << "Stability of feature " << n << " measured as " << sv[n] << endl;
#endif
		}
	}
	return sv;
}

StabilityVector TileCoding::getCardinalityStability(Episodes& episodes)
{
	StabilityVector sv;
	StabilityVector temp;
	if(!episodes.size())
		return sv;
	for(size_t n = 0 ; n < episodes.size() ; ++n)
	{
#ifdef USE_TILE_DESCRIPTIONS
		cerr << "Calculating episode " << n+1 << endl;
#endif
		temp = getCardinalityStability(*episodes[n]);
#ifdef USE_TILE_DESCRIPTIONS
		cerr << "Processing episode calculations" << endl;
#endif
		if(sv.size() < temp.size())
			sv.resize(temp.size(), 0.0);
		for(size_t i = 0 ; i < temp.size() ; ++i)
		{
			sv[i] += temp[i];
		}
	}
	for(size_t i = 0 ; i < sv.size() ; ++i)
	{
#ifdef USE_TILE_DESCRIPTIONS
		cerr << "Averaging stability of " << sv[i] << " over " << episodes.size() << " episodes\n";
#endif
		sv[i] /= episodes.size();
	}
	return sv;
}

string TileCoding::getInfo(void)
{
	char buffer[64];
	string str = "---------------------\n     Tilings Map\n---------------------\n";
	for(size_t n = 0 ; n < tilings.size() ; ++n)
	{
		sprintf(buffer, "%s (id:%d)(", tilings[n]->getName(), tilings[n]->getId());
		str += buffer;
		bool first = true;
		for(size_t i = 0 ; i < tilings[n]->getArguments().size() ; ++i)
		{
			if(!first)
				str += ", ";
			first= false;
			sprintf(buffer, "%u", tilings[n]->getArguments()[i]);
			str += buffer;
		}
		str += ")\n";
	}
	str +=       "---------------------\n     Feature Map\n---------------------\n";
	for(size_t n = 0 ; n < orderedFeatures.size() ; ++n)
	{
#ifndef USE_TILE_DESCRIPTIONS
		sprintf(buffer, "%u\t-> %u\n", n, orderedFeatures[n]);
#else
		sprintf(buffer, "%u\t-> %u\t [%s]\n", (unsigned int)n, orderedFeatures[n], descriptions[n].c_str());
#endif
		str += buffer;
	}
	return str;
}
string TileCoding::getInfo(Tiling* tiling)
{
	if(!tiling)
		return "Null feature tiling";
	
	char buffer[64];
	sprintf(buffer, "%s (id:%d)(", tiling->getName(), tiling->getId());
	string str = buffer;
	bool first = true;
	for(size_t i = 0 ; i < tiling->getArguments().size() ; ++i)
	{
		if(!first)
			str += ", ";
		first= false;
		sprintf(buffer, "%u", tiling->getArguments()[i]);
		str += buffer;
	}
	str += ")";
	
	return str;
}
string TileCoding::getQuickInfo(void)
{
	std::stringstream ss;
	ss << "Tilings       : ";
	bool first = true;
	for (size_t n = 0 ; n < tilings.size() ; n++) 
	{
		if(!first)
		{
			ss << ", ";
			ss << tilings[n]->getName();
		}
		else
		{
			first = false;
			ss << tilings[n]->getName();
		}
	}
	ss << std::endl;
	ss << "FeatureCount  : " << orderedFeatures.size();
	return ss.str();
}

std::string TileCoding::getFeatureInfo(int index)
{
	std::stringstream ss;
#ifndef USE_TILE_DESCRIPTIONS
	ss << orderedFeatures[index]);
#else
	ss <<  descriptions[index] << " [" << orderedFeatures[index] << "]";
#endif
	return ss.str();
}

#ifdef USE_TILE_DESCRIPTIONS
std::string TileCoding::getFeatureDescription(FeatureIndex i)
{
	return descriptions[i];
};
#endif


