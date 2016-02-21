#ifndef STRINGHASH_H
#define STRINGHASH_H

#include <string>

namespace cadiaplayer{
	namespace utils{
		
		// class stringhasher code by Andreas Masur 
		class StringHash
			{
			public:
				/**
				 * Required by 
				 * Inspired by the java.lang.String.hashCode() algorithm 
				 * (it's easy to understand, and somewhat processor cache-fristd::endly)
				 * @param The std::string to be hashed
				 * @return The hash value of s
				 */
				size_t operator() (const std::string& s) const
				{
					size_t h = 0;
					std::string::const_iterator p, p_end;
					for(p = s.begin(), p_end = s.end(); p != p_end; ++p)
					{
						h = 31 * h + (*p);
					}
					return h;
				}
				
				/**
				 * 
				 * @param s1 The first std::string
				 * @param s2 The second std::string
				 * @return true if the first std::string comes before the second in lexicographical order
				 */
				bool operator() (const std::string& s1, const std::string& s2) const
				{
					return s1 < s2;
				}
			};
	}} // namespaces
#endif // STRINGHASH_H
