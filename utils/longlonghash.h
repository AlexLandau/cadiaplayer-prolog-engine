/*
 *  longlonghash.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 8/21/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef LONGLONGHASH_H
#define LONGLONGHASH_H

#include <stdint.h>

namespace cadiaplayer{
	namespace utils {
		
		typedef uint64_t LongLongHashKey;
		
		struct LongLongHash
		{
			std::size_t operator()(LongLongHashKey x) const
			{ return size_t(x) + size_t(x >> 22) * 5 + size_t(x >> 43); }
			
			static std::size_t hash(LongLongHashKey x)
			{ return size_t(x) + size_t(x >> 22) * 5 + size_t(x >> 43); };
			
			bool operator()(LongLongHashKey x1, LongLongHashKey x2) const
			{ return x1 < x2; }
		};
	}} // namespaces
#endif //LONGLONGHASH_H
