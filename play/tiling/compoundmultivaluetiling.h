/*
 *  compoundmultivaluetiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef COMPOUNDMULTIVALUETILING_H
#define COMPOUNDMULTIVALUETILING_H

#include "compoundvaluetiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int COMPOUNDMULTIVALUETILING_ID = 4;
			static char COMPOUNDMULTIVALUETILING_NAME[] = "Compound Multi-Value Tiling";
			class CompoundMultiValueTiling : public CompoundValueTiling
				{
				public:
					CompoundMultiValueTiling(){};
					CompoundMultiValueTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : CompoundValueTiling(s, args){};
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c);
					virtual int getId(void){return COMPOUNDMULTIVALUETILING_ID;};
					virtual char* getName(void){return COMPOUNDMULTIVALUETILING_NAME;};
				};
		}}}
#endif // COMPOUNDMULTIVALUETILING_H
