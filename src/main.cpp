#include "pool.hpp"
#include "solution.hpp"
#include "worker.hpp"

#include <boost/program_options.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <limits>
#include <vector>

boost::program_options::variables_map parse(int argc, char* argv[]);

void printDetailedObjValue(sol::ObjValue const & objValue);

int main(int argc, char* argv[])
{
   std::cout.imbue(std::locale(""));
   std::cerr.imbue(std::locale(""));

   boost::program_options::variables_map param(parse(argc, argv));
   

   if (param.count("name") > 0)
   {
      std::cout << "J10" << std::endl;

      if (param.count("t") == 0 && param.count("p") == 0 &&
          param.count("i") == 0 && param.count("o") == 0 &&
          param.count("s") == 0)
      {
         return 0;
      }
   }

   if (param.count("t") == 0 || param.count("p") == 0 || param.count("i") == 0
       || param.count("o") == 0 || param.count("s") == 0)
   {
      std::cerr << "Error: Missing at least one parameter." << std::endl;
   }


   std::vector<Worker*> workers;
   std::vector<boost::thread*> threads;

   int numThreads = param["d"].as<int>();
   int maxNumSolutions = param["b"].as<int>();

   boost::mt19937 gen(param["s"].as<unsigned int>());
   boost::uniform_int<unsigned int> 
      dist(0, std::numeric_limits<unsigned int>::max());


   for (int i = 0; i < numThreads; i++)
   {
      workers.push_back(new Worker(param, dist(gen)));
      threads.push_back(new boost::thread(boost::ref(*(workers.back()))));
   }

   // We keep a ~5-second buffer to write the best solution
   sleep(std::max(0, param["t"].as<int>() - 5));

   for (int i = 0; i < numThreads; i++)
   {
      threads[i]->interrupt();
   }

   for (int i = 0; i < numThreads; i++)
   {
      threads[i]->join();
   }

   sol::Solution bestSolution = workers.front()->bestSolution();

   for (int i = 1; i < numThreads; i++)
   {
      sol::Solution sol = workers[i]->bestSolution();

      if (sol.objValue() < bestSolution.objValue())
      {
         bestSolution = sol;
      }
   }


   std::vector<int> const & bestAssignment = bestSolution.assignment();
   std::ofstream fileSolution(param["o"].as<std::string>().c_str());

   for (int i = 0; i < bestAssignment.size(); i++)
   {
      fileSolution << bestAssignment[i] << " ";
   }

   fileSolution.close();

   // std::cerr << "Incremental" << std::endl;
   // printDetailedObjValue(bestSolution.objValue());

   // std::cerr << std::endl << "Full" << std::endl;
   // printDetailedObjValue(bestSolution.computeObjValue());

   return 0;
}

void printDetailedObjValue(sol::ObjValue const & objValue)
{
   std::cerr << "load = " << objValue.load() << std::endl
             << "balance = " << objValue.balance() << std::endl
             << "processMove = " << objValue.processMove() << std::endl
             << "serviceMove = " << objValue.serviceMove() << std::endl
             << "machineMove = " << objValue.machineMove() << std::endl
             << "TOTAL = " << objValue.objValue() << std::endl;
}

boost::program_options::variables_map parse(int argc, char* argv[])
{
   boost::program_options::options_description desc("Allowed options");
   
   desc.add_options()
      // Mandatory parameters
      ("t", boost::program_options::value<int>(), "time limit (s)")
      ("p", boost::program_options::value<std::string>(), "model")
      ("i", boost::program_options::value<std::string>(), "initial assignment")
      ("o", boost::program_options::value<std::string>(), "solution")
      ("name", "Return the team's name")
      ("s", boost::program_options::value<unsigned int>(), "seed")
      
      // Optional parameters
      ("a", boost::program_options::value<double>()->default_value(0.01),
       "perturbation percent num moves")
      ("b", boost::program_options::value<int>()->default_value(200),
       "local search num moves")
      ("c", boost::program_options::value<int>()->default_value(200),
       "max num iter without improvement")
      ("d", boost::program_options::value<int>()->default_value(1), 
       "num threads")
      ("e", boost::program_options::value<int>()->default_value(500), 
       "local search num machines")
      ("f", boost::program_options::value<int>()->default_value(10), 
       "local search number of retries");

   boost::program_options::variables_map param;

   boost::program_options::store(
      boost::program_options::command_line_parser(argc, argv).
      options(desc).style(
         boost::program_options::command_line_style::default_style |
         boost::program_options::command_line_style::allow_long_disguise).run(),
      param);
   
   boost::program_options::notify(param);

   return param;
}
