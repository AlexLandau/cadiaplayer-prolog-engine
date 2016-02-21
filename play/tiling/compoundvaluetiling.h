/*
 *  compoundvaluetiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef COMPOUNDVALUETILING_H
#define COMPOUNDVALUETILING_H

#include "valuetiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int COMPOUNDVALUETILING_ID = 3;
			static char COMPOUNDVALUETILING_NAME[] = "Compound Value Tiling";
			class CompoundValueTiling : public ValueTiling
				{
				public:
					CompoundValueTiling(){};
					CompoundValueTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : ValueTiling(s, args){};
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c)
					{return index < c->getArguments()->size() ? 
						(symbols->getName(c->getName()) + "(" + c->getArgument(index)->toString(symbols) + ")")
					: "[n/a]" ;};
					virtual int getId(void){return COMPOUNDVALUETILING_ID;};
					virtual char* getName(void){return COMPOUNDVALUETILING_NAME;};
				};
		}}}
#endif // COMPOUNDVALUETILING_H
