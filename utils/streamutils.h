/*
 *  streamutils.h
 *  cadiaplayer
 *
 *  Created by Stephan Schiffel on 11/03/17.
 *  Copyright 2011 Reykjavik University. All rights reserved.
 *
 */

#ifndef STREAMUTILS_H
#define STREAMUTILS_H

#include <iostream>
#include <vector>

namespace std
{

	// override << operator to output vectors
	template <class T> ostream& operator<<(ostream& out, const vector<T> &v)
	{
		out << '[';
		typename vector<T>::const_iterator it = v.begin();
		if (it != v.end()) {
			out << *it;
			++it;
			for(; it != v.end(); ++it) {
				out << ", " << *it;
			}
		}
		out << ']';
		return out;
	}

	
} // namespaces
#endif //STREAMUTILS_H
