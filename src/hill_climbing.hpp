#ifndef HILL_CLIMBING_HPP
#define HILL_CLIMBING_HPP

#include "instance.hpp"
#include "pool.hpp"
#include "solution.hpp"

#include <algorithm>
#include <limits>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/thread/thread.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

class HillClimbing
{
public:
   HillClimbing(unsigned int seed, inst::Instance const & instance, 
                Pool * pool, int numProcesses, int numMachines,
                int numTriesMax)
      : _inst(instance),
        _pool(pool),
        _numMachines(0),
        _numProcesses(0),
        _machines(boost::counting_iterator<int>(0),
                  boost::counting_iterator<int>(_inst.numMachines())),
        _processes(boost::counting_iterator<int>(0),
                   boost::counting_iterator<int>(_inst.numProcesses())),
        _gen(seed),
        _rng(_gen, _dist),
        _numTriesMax(numTriesMax)
   {
      setNumMachines(numMachines);
      setNumProcesses(numProcesses);
   }

   sol::Solution apply(sol::Solution const & solution)
   {
      inst::integer bestValue;
      std::pair<int, int> bestMove;
      sol::ObjValue bestDeltaObjValue;
      sol::Solution currentSolution(solution);
      int numTries = 0;

      do
      {
         bestValue = std::numeric_limits<inst::integer>::max();

         shuffleProcesses();

         for (int i = 0; i < _numProcesses; i++)
         {
            int process = _processes[i];

            shuffleMachines();

            for (int j = 0; j < _numMachines; j++)
            {
               int machine = _machines[j];

               if (currentSolution.assignment()[process] == machine)
                  continue;

               if (!currentSolution.isFeasible(process, machine))
                  continue;
               
               sol::ObjValue deltaObjValue(
                  currentSolution.evaluateFeasibleMove(process, machine));

               inst::integer value = deltaObjValue.objValue();
               
               if (value < bestValue)
               {
                  bestValue = value;
                  bestMove = std::make_pair(process, machine);
                  bestDeltaObjValue = deltaObjValue;
               }
            }
         }

         if (bestValue < 0)
         {
            currentSolution.moveProcess(bestMove.first, bestMove.second,
                                  bestDeltaObjValue);
            _pool->addSolution(currentSolution);
            numTries = 0;
         }
         else
         {
            numTries++;
         }

         boost::this_thread::interruption_point();
      }
      while(bestValue < 0 || numTries < _numTriesMax);


      return currentSolution;
   }

   void setNumMachines(int numMachines)
   {
      _numMachines = std::min(numMachines, _inst.numMachines());
   }

   void setNumProcesses(int numProcesses)
   {
      _numProcesses = std::min(numProcesses, _inst.numProcesses());
   }


private:

   void shuffleMachines()
   {
      std::random_shuffle(_machines.begin(), _machines.end(), _rng);
   }

   void shuffleProcesses()
   {
      std::random_shuffle(_processes.begin(), _processes.end(), _rng);
   }

   inst::Instance const & _inst;
   Pool * _pool;
   int _numMachines;
   int _numProcesses;

   std::vector<int> _machines;
   std::vector<int> _processes;

   boost::mt19937 _gen;
   boost::uniform_int<> _dist;

   boost::variate_generator<boost::mt19937&, boost::uniform_int<> > _rng;

   int _numTriesMax;
};

#endif
