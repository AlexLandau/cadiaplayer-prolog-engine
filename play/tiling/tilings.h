/*
 *  tilings.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/30/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef TILINGS_H
#define TILINGS_H

#include "compoundtiling.h"
#include "valuetiling.h"
#include "compoundvaluetiling.h"
#include "compoundmultivaluetiling.h"
#include "rolepiecetiling.h"
#include "piecevaluetiling.h"

namespace cadiaplayer {
	namespace play {
		namespace tiling {
			
			class Tilings
				{
				public:
					static Tiling* getTilingById(int id, cadiaplayer::play::parsing::SymbolTable* s, TileArguments args)
					{
						Tiling* tiling = NULL;
						switch(id)
						{
							case COMPOUNDTILING_ID : 
								tiling = new CompoundTiling(s, args);break;
							case VALUETILING_ID : 
								tiling = new ValueTiling(s, args);break;
							case COMPOUNDVALUETILING_ID : 
								tiling = new CompoundValueTiling(s, args);break;
							case COMPOUNDMULTIVALUETILING_ID : 
								tiling = new CompoundMultiValueTiling(s, args);break;
							case ROLEPIECETILING_ID : 
								tiling = new RolePieceTiling(s, args);break;
							case PIECEVALUETILING_ID : 
								tiling = new PieceValueTiling(s, args);break;
							default : 
								tiling = new CompoundTiling(s, args);break;
						}
						return tiling;
					};
				};
		}}}

#endif //TILINGS_H
