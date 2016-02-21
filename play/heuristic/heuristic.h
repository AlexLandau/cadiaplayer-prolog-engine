#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "../gametheory.h"
#include "../players/gameplayer.h"

namespace cadiaplayer {
	namespace play {
		namespace heuristic {
			
			typedef std::vector<double> Weights;
			
			class Heuristic
				{
				public:
					Weights weights;
					virtual ~Heuristic(){};
					virtual bool generate(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex /*r*/, cadiaplayer::play::players::GamePlayer* /*player*/)
					{
						return false;
					};
					virtual double getHeuristic(cadiaplayer::play::GameTheory& theory, cadiaplayer::play::RoleIndex r, cadiaplayer::play::players::GamePlayer* /*player*/)
					{
						return theory.goal(r);
					};
					virtual void setWeights(Weights /*w*/){};
					virtual Weights getWeights(void)
					{
						Weights w;
						return w;
					};
					virtual void reverse()
					{
						for(Weights::iterator it  = weights.begin() ; it != weights.end() ; ++it)
						{
							(*it) *= -1;
						}
					};
				};
		}}} // namespaces
#endif // HEURISTIC_H
