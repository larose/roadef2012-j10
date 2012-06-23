#include "parser.hpp"

#include <algorithm>
#include <iostream>
#include <map>

inst::Instance* Parser::parse(std::ifstream & file,
                              std::ifstream & fileAssignment)
{
   std::vector<inst::Resource> resources;
   std::vector<inst::Machine> machines;
   std::vector<inst::Service> services;
   std::vector<inst::Process> processes;
   std::vector<inst::BalanceCost> balanceCosts;
   std::vector<int> initAssignment;
   int processMoveCostWeight;
   int serviceMoveCostWeight;
   int machineMoveCostWeight;

   int numNeighborhoods;
   int numLocations;
   
   parseRessources(file, resources);
   parseMachines(file, resources.size(), machines, &numNeighborhoods,
                 &numLocations);
   parseServices(file, services);
   parseProcesses(file, resources.size(), processes);
   parseBalanceCosts(file, balanceCosts);
   parseWeights(file, processMoveCostWeight, serviceMoveCostWeight,
                machineMoveCostWeight);
   parseSolution(fileAssignment, &initAssignment);

   inst::Instance* instance = new inst::Instance(
      resources,
      machines,
      services,
      processes,
      balanceCosts,
      initAssignment,
      processMoveCostWeight,
      serviceMoveCostWeight,
      machineMoveCostWeight,
      numNeighborhoods,
      numLocations);

   return instance;
}

void Parser::parseRessources(std::ifstream& file,
                             std::vector<inst::Resource>& resources)
{
   int numRessources;

   file >> numRessources;

   for (int i = 0; i < numRessources; i++)
   {
      bool transient;
      int loadCostWeight;

      file >> transient;
      file >> loadCostWeight;

      resources.push_back(inst::Resource(i, transient, loadCostWeight));
   }
}

void Parser::parseMachines(std::ifstream& file, int numResources,
                           std::vector<inst::Machine>& machines,
                           int * numNeighborhoods,
                           int * numLocations)
{
   int numMachines;

   file >> numMachines;

   std::map<int, int> neighborhoods;
   std::map<int, int> locations;

   for (int i = 0; i < numMachines; i++)
   {
      int neighborhood;
      int location;
      std::vector<inst::integer> capacities;
      std::vector<inst::integer> safetyCapacities;
      std::vector<inst::integer> moveCosts;

      file >> neighborhood;
      file >> location;

      //////////////////////////////////////////////////////////////////////
      // Ensure that neighborhoods and locations are indexed sequentially.

      std::map<int, int>::const_iterator neighIt
         = neighborhoods.find(neighborhood);

      int neighborhoodIndex;
      
      if (neighIt != neighborhoods.end())
      {
         neighborhoodIndex = neighIt->second;
      }
      else
      {
         int size = neighborhoods.size();
         neighborhoods[neighborhood] = size;
         neighborhoodIndex = size;
      }

      int locationIndex;
      
      std::map<int, int>::const_iterator locationIt
         = locations.find(location);

      if (locationIt != locations.end())
      {
         locationIndex = locationIt->second;
      }
      else
      {
         int size = locations.size();
         locations[location] = size;
         locationIndex = size;
      }
 
      fillArray(file, numResources, std::back_inserter(capacities));
      fillArray(file, numResources, std::back_inserter(safetyCapacities));
      fillArray(file, numMachines,  std::back_inserter(moveCosts));

      machines.push_back(inst::Machine(i, neighborhood, location,
                                           capacities, safetyCapacities,
                                           moveCosts));
   }

   *numNeighborhoods = neighborhoods.size();
   *numLocations = locations.size();
}


void Parser::parseServices(std::ifstream& file,
                           std::vector<inst::Service>& services)
{
   int numServices;

   file >> numServices;

   std::vector<std::vector<int> > reverseDependencies(numServices);

   for (int i = 0; i < numServices; i++)
   {
      int spreadMin;
      int numDependencies;
      std::vector<int> dependencies;
      
      file >> spreadMin;
      file >> numDependencies;

      fillArray(file, numDependencies, std::back_inserter(dependencies));

      for (int j = 0; j < dependencies.size(); j++)
      {
         reverseDependencies[dependencies[j]].push_back(i);
      }

      services.push_back(inst::Service(i, spreadMin, dependencies));
   }

   for (int i = 0; i < numServices; i++)
   {
      services[i].setReverseDependencies(reverseDependencies[i]);
   }
}

void Parser::parseSolution(std::ifstream& file,
                           std::vector<int> * assignment)
{
   int machine;
   while (file >> machine)
   {
      assignment->push_back(machine);
   }
}

void Parser::parseProcesses(std::ifstream& file, int numResources,
                            std::vector<inst::Process>& processes)
{
   int numProcesses;

   file >> numProcesses;

   for (int i = 0; i < numProcesses; i++)
   {
      int service;
      std::vector<inst::integer> requirements;
      int moveCost;

      file >> service;

      fillArray(file, numResources, std::back_inserter(requirements));

      file >> moveCost;

      processes.push_back(inst::Process(i, service, requirements, moveCost));

   }
}

void Parser::parseBalanceCosts(std::ifstream& file,
                               std::vector<inst::BalanceCost>& balanceCosts)
{
   int numBalanceCosts;

   file >> numBalanceCosts;

   for (int i = 0; i < numBalanceCosts; i++)
   {
      int firstResource;
      int secondResource;
      int target;
      int weight;

      file >> firstResource;
      file >> secondResource;
      file >> target;
      file >> weight;

      balanceCosts.push_back(inst::BalanceCost(i, firstResource,
                                                   secondResource,
                                                   target, weight));
   }
}

void Parser::parseWeights(std::ifstream& file, int& processMoveCostWeight,
                          int& serviceMoveCostWeight,
                          int& machineMoveCostWeight)
{
   file >> processMoveCostWeight;
   file >> serviceMoveCostWeight;
   file >> machineMoveCostWeight;
}


