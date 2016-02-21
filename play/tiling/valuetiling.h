/*
 *  valuetiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef VALUETILING_H
#define VALUETILING_H

#include "tiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int VALUETILING_ID = 2;
			static char VALUETILING_NAME[] = "Value Tiling";
			class ValueTiling : public Tiling
				{
				public:
					unsigned int index;
					ValueTiling(){};
					ValueTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : Tiling(s, args) 
					{if(!args.size())index = 0;else index = args[0];};
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c)
					{return index < c->getArguments()->size() ? c->getArgument(index)->toString(symbols) : "[n/a]" ;};
					virtual int getId(void){return VALUETILING_ID;};
					virtual char* getName(void){return VALUETILING_NAME;};
				};
		}}}

#endif // VALUETILING_H
