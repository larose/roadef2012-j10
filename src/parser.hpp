#ifndef PARSER_HPP
#define PARSER_HPP

#include "instance.hpp"
#include "solution.hpp"

#include <fstream>
#include <iterator>

class Parser
{
public:
   static inst::Instance* parse(std::ifstream& file,
                                std::ifstream & fileAssignment);
private:
   template <class OutputIterator>
   static void fillArray(std::ifstream& file, int size, OutputIterator result);


   static void parseRessources(std::ifstream& file,
                               std::vector<inst::Resource>& resources);
   static void parseMachines(std::ifstream& file, int numResources,
                             std::vector<inst::Machine>& machines,
                             int* numNeighborhoods,
                             int* numLocations);
   static void parseServices(std::ifstream& file,
                             std::vector<inst::Service>& services);
   static void parseSolution(std::ifstream& file,
                             std::vector<int> * assignment);
   static void parseProcesses(std::ifstream& file, int numResources,
                              std::vector<inst::Process>& processes);
   static void parseBalanceCosts(
      std::ifstream& file,
      std::vector<inst::BalanceCost>& balanceCosts);
   static void parseWeights(std::ifstream& file, int& processMoveCostWeight,
                            int& serviceMoveCostWeight,
                            int& machineMoveCostWeight);
};

template <class OutputIterator>
void Parser::fillArray(std::ifstream& file, int size, OutputIterator result)
{
   for (int i = 0; i < size; i++)
   {
      int value;

      file >> value;

      *result = value;
   }
}


#endif
