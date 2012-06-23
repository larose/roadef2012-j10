#ifndef EXECUTE_HPP
#define EXECUTE_HPP

#include "hill_climbing.hpp"
#include "instance.hpp"
#include "iterated_ls.hpp"
#include "parser.hpp"
#include "pool.hpp"
#include "random_moves.hpp"
#include "solution.hpp"

#include <boost/program_options.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>


class Worker
{
public:
   Worker(boost::program_options::variables_map const & param, 
          unsigned int seed)
      : _param(param),
        _pool(1),
        _gen(seed)
   {
   }
   
   void operator()()
   {
      // Memory leak at the end...
      inst::Instance* instance = createInstance(_param);

      sol::Solution initialSolution(instance);
      sol::ObjValue initObjValue
         = initialSolution.computeObjValue();
      initialSolution.applyDelta(initObjValue);

      _pool.addSolution(initialSolution);

      RandomMoves randomMoves(
         _dist(_gen),
         *instance,
         instance->numProcesses() * _param["a"].as<double>());
      
      HillClimbing hillClimbing(_dist(_gen), *instance, &_pool, 
                                _param["b"].as<int>(),
                                _param["e"].as<int>(),
                                _param["f"].as<int>());
      
      IteratedLocalSearch<HillClimbing, RandomMoves>
         ils(_param["c"].as<int>(),
             &hillClimbing,
             &randomMoves,
             &_pool);

      do
      {
         sol::Solution solution(bestSolution());
         ils.apply(solution);
         boost::this_thread::interruption_point();
      }
      while(true);
   }

   sol::Solution bestSolution() const
   {
      return _pool.getBestSolution();
   }
   
private:

   static inst::Instance* createInstance(
      boost::program_options::variables_map const & param)
   {
      std::ifstream instanceFile(param["p"].as<std::string>().c_str());
      std::ifstream initialSolutionFile(param["i"].as<std::string>().c_str());
      return Parser::parse(instanceFile, initialSolutionFile);
   }

   boost::program_options::variables_map const & _param;
   Pool _pool;
   boost::mt19937 _gen;
   boost::uniform_int<unsigned int> _dist;
};

#endif
