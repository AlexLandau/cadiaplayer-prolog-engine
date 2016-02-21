/*
 *  strutils.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 11/1/10.
 *  Copyright 2010 Reykjavik University. All rights reserved.
 *
 */

#ifndef STRUTILS_H
#define STRUTILS_H

#include <string>
#include <vector>

namespace cadiaplayer
{
	namespace utils
	{
		
		class StringUtils
		{
		public:
		
			static std::string replaceAll(std::string str, std::string find, std::string replace)
			{
				std::string res = str;
				int position = res.find( find ); // find first space
				int flen = find.length();
				int rlen = replace.length();
				// 
				while ( position != std::string::npos ) 
				{
					res.replace( position, flen, replace );
					position = res.find( find, position + rlen );
				}
				return res;
			};
			
			static std::string trim(std::string str)
			{
				std::string res = replaceAll(str, " ", "");
				res = replaceAll(str, "\t", "");
				res = replaceAll(str, "\n", "");
				
				return res;
			};
			
			static void tokenize(const std::string& str,
						  std::vector<std::string>& tokens,
						  const std::string& delimiters = " ")
			{
				// Skip delimiters at beginning.
				std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
				// Find first "non-delimiter".
				std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
				
				while (std::string::npos != pos || std::string::npos != lastPos)
				{
					// Found a token, add it to the vector.
					tokens.push_back(str.substr(lastPos, pos - lastPos));
					// Skip delimiters.  Note the "not_of"
					lastPos = str.find_first_not_of(delimiters, pos);
					// Find next "non-delimiter"
					pos = str.find_first_of(delimiters, lastPos);
				}
			};
			
			static void tokenize(const std::string& str,
								 std::vector<std::string>& tokens,
								 const char delimiter = ' ')
			{
				// Skip delimiters at beginning.
				std::string::size_type lastPos = str.find_first_not_of(delimiter, 0);
				// Find first "non-delimiter".
				std::string::size_type pos     = str.find_first_of(delimiter, lastPos);
				
				while (std::string::npos != pos || std::string::npos != lastPos)
				{
					// Found a token, add it to the vector.
					tokens.push_back(str.substr(lastPos, pos - lastPos));
					// Skip delimiters.  Note the "not_of"
					lastPos = str.find_first_not_of(delimiter, pos);
					// Find next "non-delimiter"
					pos = str.find_first_of(delimiter, lastPos);
				}
			};
		};
	}
} // namespaces
#endif //STRUTILS_H
