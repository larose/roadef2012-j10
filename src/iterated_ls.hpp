#ifndef ITERATED_LS_HPP
#define ITERATED_LS_HPP

#include "instance.hpp"
#include "pool.hpp"
#include "solution.hpp"

#include <cmath>
#include <limits>

template <typename LocalSearch, typename Perturbation>
class IteratedLocalSearch
{
public:
   IteratedLocalSearch(int maxNumNonImprovIter,
                       LocalSearch *  localSearch,
                       Perturbation * perturbation,
                       Pool * pool)
      : _maxNumNonImprovIter(maxNumNonImprovIter),
        _localSearch(localSearch),
        _perturbation(perturbation),
        _pool(pool)
   {
   }

   void apply(sol::Solution const & solution)
   {
      int numIter = 0;
      int lastBestIter = -1;

      sol::Solution bestSolution(solution);
      sol::Solution currentSolution(_localSearch->apply(solution));

      if (isBetter(currentSolution, bestSolution))
      {
         lastBestIter = 0;
         bestSolution = currentSolution;
      }
      
      do
      {
         currentSolution = _perturbation->apply(currentSolution);
         currentSolution = _localSearch->apply(currentSolution);

         _pool->addSolution(currentSolution);

         if (isBetter(currentSolution, bestSolution))
         {
            lastBestIter = numIter;
            bestSolution = currentSolution;
         }

         numIter++;
         
         boost::this_thread::interruption_point();
      }
      while ((numIter - lastBestIter) <= _maxNumNonImprovIter);
   }
   
private:
   bool isBetter(sol::Solution const & sol1,
                 sol::Solution const & sol2) const
   {
      inst::integer value1 = sol1.objValue().objValue();
      inst::integer value2 = sol2.objValue().objValue();
      
      return value1 < value2;
   }

   int _maxNumNonImprovIter;
   LocalSearch * _localSearch;
   Perturbation * _perturbation;
   Pool * _pool;
};

#endif
