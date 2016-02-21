/*
 *  compoundtiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef COMPOUNDTILING_H
#define COMPOUNDTILING_H

#include "tiling.h"

#define COMPOUNDTILING_SHIFT_SIZE 	9
#define COMPOUNDTILING_SHIFT_BOUND 	24

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int COMPOUNDTILING_ID = 1;
			static char COMPOUNDTILING_NAME[] = "Compound Tiling";
			class CompoundTiling : public Tiling
				{
				public:
					CompoundTiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args) : Tiling(s, args){};
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* c);
					virtual int getId(void){return COMPOUNDTILING_ID;};
				private:
					TileKey encodeArgs(cadiaplayer::play::parsing::Compound* c, unsigned int& shift);
					TileKey encodeCompound(cadiaplayer::play::parsing::Compound* c, unsigned int& shift);
					virtual char* getName(void){return COMPOUNDTILING_NAME;};
				};
		}}}

#endif // COMPOUNDTILING_H
