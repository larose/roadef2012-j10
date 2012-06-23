#ifndef POOL_HPP
#define POOL_HPP

#include "solution.hpp"

#include <algorithm>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/thread/mutex.hpp>
#include <list>

class Pool
{
public:

   class NoSolution {};

   Pool(int maxNumSolutions)
      : _maxNumSolutions(maxNumSolutions)
   {
   }

   void addSolution(sol::Solution const & solution)
   {
      if (_pool.size() < _maxNumSolutions)
      {
         insertSolution(solution);
      }
      else if (solution.objValue().objValue()
               < _pool.back().objValue().objValue())
      {
         bool inserted = insertSolution(solution);

         if (inserted)
            removeWorst();
      }
   }

   sol::Solution getBestSolution() const
   {
      if (_pool.empty())
         throw NoSolution();

      return _pool.front();
   }

        
private:

   bool insertSolution(sol::Solution const & solution)
   {
      int index = 0;
      for (std::list<sol::Solution>::iterator it = _pool.begin();
           it != _pool.end();
           ++it)
      {
         if (solution.objValue().objValue()
             == it->objValue().objValue())
         {
            return false;
         }
         
         if (solution.objValue().objValue()
             < it->objValue().objValue())
         {
            _pool.insert(it, sol::Solution(solution));
            return true;
         }
         index++;
      }
      
      _pool.push_back(solution);
      return true;
   }
      
   void removeWorst()
   {
      _pool.pop_back();
   }

   int _maxNumSolutions;
   std::list<sol::Solution> _pool;
};

#endif
