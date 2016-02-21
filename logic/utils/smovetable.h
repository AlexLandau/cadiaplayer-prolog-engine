/*
 *  SMoveTable.h
 *  client
 *
 *  Created by Hilmar Finnsson on 5/2/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#ifndef INCL_SMOVETABLE_H
#define INCL_SMOVETABLE_H

#include <map>
#include <stack>
#include <string>

namespace cadiaplayer {
	namespace logic {
		namespace utils {
			
			const double DEFAULT_Q_VALUE = 100;
			
			struct SMoveTableItem
			{
				unsigned int n;
				double q;
				SMoveTableItem(){n=0;q=0.0;};
				void set(double q_val, unsigned int n_val){q=q_val;n=n_val;};
			};
			
			class SMoveTable
				{
				private:
					std::map<std::string, SMoveTableItem> table;
					std::stack< std::string > path;
				public:
					SMoveTable(){};
					size_t size(){return table.size();};
					void add(const std::string& str, const double& qVal)
					{
						SMoveTableItem& item = table[str];
						item.n++;
						item.q += (1.0/item.n)*(qVal - item.q);
					};
					double get(const std::string& str)
					{
						/*std::map<std::string, SMoveTableItem>::iterator itr = table.find(str);
						 if(itr == table.end())
						 return DEFAULT_Q_VALUE;
						 if(itr->second.n == 0)
						 return DEFAULT_Q_VALUE;
						 return itr->second.q;*/
						SMoveTableItem& item = table[str];
						if(item.n == 0)
							return DEFAULT_Q_VALUE;
						return item.q;
					};
					void set(const std::string& str, const double& qVal, const unsigned int& nVal)
					{
						SMoveTableItem& item = table[str];
						item.set(qVal, nVal);
					};
					void setIfBetter(const std::string& str, const double& qVal, const unsigned int& nVal)
					{
						SMoveTableItem& item = table[str];
						if(item.n < nVal)
							item.set(qVal, nVal);
					};
					void addToPath(std::string move)
					{
						path.push(move);
					}
					void evaluatePath(double goal, double gamma)
					{
						while(!path.empty())
						{
							add(path.top(), goal);
							goal*=gamma;
							path.pop();
						};
					}
					void str(std::stringstream& ss)
					{
						size_t count;
						ss >> count;
						std::string action;
						double q_val;
						unsigned int n_val;
						char space;
						for(size_t n = 0 ; n < count ; n++)
						{
							ss >> q_val >> n_val;
							ss.get(space);
							getline(ss, action);
							setIfBetter(action, q_val, n_val);
						}
					}
					std::string str()
					{
						std::stringstream ss;
						ss << table.size() << std::endl;
						for(std::map<std::string, SMoveTableItem>::iterator itr = table.begin() ; itr != table.end() ; itr++)
						{
							ss << itr->second.q << " " << itr->second.n << " " << itr->first << std::endl;
						}
						return ss.str();
					};
					std::string toString()
					{
						std::stringstream ss;
						for(std::map<std::string, SMoveTableItem>::iterator itr = table.begin() ; itr != table.end() ; itr++)
						{
							ss << itr->first << " : " << itr->second.q << " (" << itr->second.n << ")" << std::endl;
						}
						return ss.str();
					};
				};
			
			typedef std::vector<SMoveTable> SMoveTables;
			typedef SMoveTables::iterator SMoveTablesItr;
			
		}}} // endif
#endif
