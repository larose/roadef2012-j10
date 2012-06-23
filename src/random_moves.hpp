#ifndef RANDOM_MOVES_HPP
#define RANDOM_MOVES_HPP

#include "instance.hpp"
#include "solution.hpp"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/thread/thread.hpp>

class RandomMoves
{
public:
   RandomMoves(unsigned int seed, inst::Instance const & instance, int numMoves)
      : _numMoves(numMoves),
        _gen(seed),
        _distMachine(0, instance.numMachines() - 1),
        _distProcess(0, instance.numProcesses() - 1)
   {
   }

   sol::Solution apply(sol::Solution const & solution)
   {
      sol::Solution currentSolution(solution);
         
      int i = 0;
      int numMovedProcess = 0;

      do
      {
         int process = _distProcess(_gen);
         int machine = _distMachine(_gen);

         if (currentSolution.isFeasible(process, machine))
         {

            sol::ObjValue
               deltaObjValue(
                  currentSolution.evaluateFeasibleMove(process, machine));

            currentSolution.moveProcess(process, machine, deltaObjValue);
            numMovedProcess++;
         }

         i++;
         boost::this_thread::interruption_point();
      }
      while (numMovedProcess < _numMoves && i < 1000);

      return currentSolution;
   }

private:
   int _numMoves;
   boost::mt19937 _gen;
   boost::uniform_int<> _distMachine;
   boost::uniform_int<> _distProcess;
};

#endif
