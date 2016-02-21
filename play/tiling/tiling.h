/*
 *  tiling.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef TILING_H
#define TILING_H

#include "../parsing/compound.h"
#include "../heuristic/heuristic.h"
#include <vector>

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			const int TILING_ID =  0;
			static char TILING_NAME[] =  "Abstract Tiling";

			typedef unsigned int TileKey;
			typedef unsigned int TileArgument;
			typedef std::vector<TileArgument> TileArguments;
			
			const TileKey FEATURE_NONE = 0;
			const TileKey TileKeySize = sizeof(TileKey)*8;
			const TileKey HalfTileKeySize = sizeof(TileKey)*4;
			const TileKey QuartTileKeySize = sizeof(TileKey)*2;
						
			class Tiling
				{
				public:
					TileArguments arguments;
					cadiaplayer::play::parsing::SymbolTable* symbols;
					Tiling(){};
					Tiling(cadiaplayer::play::parsing::SymbolTable* s, TileArguments& args)
					{symbols = s; arguments.insert(arguments.end(), args.begin(), args.end());};
					virtual ~Tiling(){};
					TileArguments& getArguments(void){return arguments;};
					virtual TileKey getFeature(cadiaplayer::play::parsing::Compound* /*c*/){return FEATURE_NONE;};
					virtual TileKey getFeature(cadiaplayer::play::parsing::CompoundList& cList)
					{
						TileKey join = FEATURE_NONE;
						for(size_t n = 0 ; n < cList.size() ; ++n)
						{
							join = join^(getFeature(cList[n]));
						}
						return join;
					};
					virtual std::string toString(cadiaplayer::play::parsing::Compound* c){return c->toString(symbols);};
					virtual int getId(void){return TILING_ID;};
					virtual char* getName(void){return TILING_NAME;};
				};
		}}}

#endif // TILING_H
