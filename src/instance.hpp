#ifndef MODEL_HPP
#define MODEL_HPP

#include <set>
#include <vector>

namespace inst
{
   typedef long long int integer;

   class Resource
   {
   public:
      Resource(int id, bool transient, int loadCostWeight)
         : _id(id),
           _transient(transient),
           _loadCostWeight(loadCostWeight)
      {
      }

      int id() const { return _id; }
      bool transient() const { return _transient; }
      int loadCostWeight() const { return _loadCostWeight; }

   private:
      int _id;
      bool _transient;
      int _loadCostWeight;
   };

   class Machine
   {
   public:

      Machine(int id, int neighborhood, int location,
              const std::vector<integer>& capacities,
              const std::vector<integer>& safetyCapacities,
              const std::vector<integer>& moveCost)
         : _id(id),
           _neighborhood(neighborhood),
           _location(location),
           _capacities(capacities),
           _safetyCapacities(safetyCapacities),
           _moveCosts(moveCost)
      {
      }

      int id() const { return _id; }
      int neighborhood() const { return _neighborhood; }
      int location() const { return _location; }
      int capacity(int resource) const { return _capacities[resource]; }
      std::vector<integer> const & capacities() const { return _capacities; }
      int safetyCapacity(int resource) const
      { return _safetyCapacities[resource]; }
      int moveCost(int machine) const { return _moveCosts[machine]; }

      std::vector<integer> const & safetyCapacities() const
      { return _safetyCapacities; }


   private:
      int _id;
      int _neighborhood;
      int _location;
      std::vector<integer> _capacities;
      std::vector<integer> _safetyCapacities;
      std::vector<integer> _moveCosts;
   };

   class Service
   {
   public:
      Service(int id, int spreadMin, std::vector<int> dependencies)
         : _id(id),
           _spreadMin(spreadMin),
           _dependencies(dependencies)
      {
      }

      void addProcess(int process) { _processes.insert(process); }

      void setReverseDependencies(const std::vector<int>&
                                  reverseDependencies)
      { _reverseDependencies = reverseDependencies; }
   
      int id() const { return _id; }
      int spreadMin() const { return _spreadMin; }

      std::vector<int> const & dependencies() const { return _dependencies; }
      std::set<int> const & processes() const { return _processes; }
      std::vector<int> const & reverseDependencies() const
      { return _reverseDependencies; }

   private:
      int _id;
      int _spreadMin;
      std::vector<int> _dependencies;
      std::vector<int> _reverseDependencies;
      std::set<int> _processes;
   };

   class Process
   {
   public:

      Process(int id, int service, const std::vector<integer>& requirements,
              int moveCost)
         : _id(id),
           _service(service),
           _requirements(requirements),
           _moveCost(moveCost)
      {
      }

      int id() const { return _id; }
      int service() const { return _service; }
      int moveCost() const { return _moveCost; }
      integer requirement(int resource) const
      { return _requirements[resource]; }

      std::vector<integer> const & requirements() const
      { return  _requirements; }


   private:
      int _id;
      int _service;
      std::vector<integer> _requirements;
      int _moveCost;
   };

   class BalanceCost
   {
   public:
      BalanceCost(int id, int firstResource, int secondResource,
                  int target, int weight)
         : _id(id),
           _firstResource(firstResource),
           _secondResource(secondResource),
           _target(target),
           _weight(weight)
      {
      }

      int id() const { return _id; }
      int firstResource() const { return _firstResource; }
      int secondResource() const { return _secondResource; }
      int target() const { return _target; }
      int weight() const { return _weight; }   

   private:
      int _id;
      int _firstResource;
      int _secondResource;
      int _target;
      int _weight;
   };

   class Location
   {
   public:
      Location(int id)
         : _id(id)
      {
      }

      void addMachine(int machine) { _machines.insert(machine); }
      std::set<int> const & machines() const { return _machines; }
      
   private:
      int _id;
      std::set<int> _machines;
   };

   class Neighborhood
   {
   public:
      Neighborhood(int id)
         : _id(id)
      {
      }

      void addMachine(int machine) { _machines.insert(machine); }
      std::set<int> const & machines() const { return _machines; }
      
   private:
      int _id;
      std::set<int> _machines;
   };

   class Instance
   {
   public:
      Instance(const std::vector<Resource>& resources,
               const std::vector<Machine>& machines,
               const std::vector<Service>& services,
               const std::vector<Process>& processes,
               const std::vector<BalanceCost>& balanceCosts,
               std::vector<int> const & initAssignment,
               int processMoveCostWeight,
               int serviceMoveCostWeight,
               int machineMoveCostWeight,
               int numNeighborhoods,
               int numLocations)
         : _resources(resources),
           _machines(machines),
           _services(services),
           _processes(processes),
           _balanceCosts(balanceCosts),
           _initAssignment(initAssignment),
           _processMoveCostWeight(processMoveCostWeight),
           _serviceMoveCostWeight(serviceMoveCostWeight),
           _machineMoveCostWeight(machineMoveCostWeight),
           _numNeighborhoods(numNeighborhoods),
           _numLocations(numLocations),
           _numResources(_resources.size()),
           _resourcesLoadCostWeight(_resources.size(), 0)
      {
         for (int i = 0; i < numResources(); i++)
         {
            _isTransient.push_back(_resources[i].transient());
            _resourcesLoadCostWeight[i] = _resources[i].loadCostWeight();
         }

         for (int i = 0; i < _processes.size(); i++)
         {
            _services[_processes[i].service()].addProcess(i);
         }

         for (int i = 0; i < _numLocations; i++)
         {
            _locations.push_back(Location(i));
         }

         for (int i = 0; i < _numNeighborhoods; i++)
         {
            _neighborhoods.push_back(Neighborhood(i));
         }

         for (int i = 0; i < _machines.size(); i++)
         {
            _locations[machines[i].location()].addMachine(i);
            _neighborhoods[machines[i].neighborhood()].addMachine(i);
         }

         for (int i = 0; i < _services.size(); i++)
         {
            for (int j = 0; j < _services[i].dependencies().size(); j++)
            {
               _dependencies.push_back(
                  std::make_pair(i, _services[i].dependencies()[j]));
            }
         }

      }

      int numBalanceCosts() const { return _balanceCosts.size(); }
      int numDependencies() const { return _dependencies.size(); }
      int numLocations() const { return _numLocations; }
      int numMachines() const { return _machines.size(); }
      int numNeighborhoods() const { return _numNeighborhoods; }
      int numProcesses() const { return _processes.size(); }
      int numResources() const { return _numResources; }
      int numServices() const { return _services.size(); }

      BalanceCost const & balanceCost(int balanceCost) const
      { return _balanceCosts[balanceCost]; }
      
      const std::vector<BalanceCost>& balanceCosts() const
      { return _balanceCosts; }

      std::pair<int, int> const & dependency(int dependency) const
      { return _dependencies[dependency]; }

      Location const & location(int location) const
      { return _locations[location]; }
      
      const Machine& machine(int machine) const { return _machines[machine]; }

      Neighborhood const & neighborhood(int neighborhood) const
      { return _neighborhoods[neighborhood]; }

      const Process& process(int process) const
      { return _processes[process]; }
      
      const Resource& resource(int resource) const
      { return _resources[resource]; }


      const Service& service(int service) const { return _services[service]; }

      int processMoveCostWeight() const { return _processMoveCostWeight; }
      int serviceMoveCostWeight() const { return _serviceMoveCostWeight; }
      int machineMoveCostWeight() const { return _machineMoveCostWeight; }

      std::vector<unsigned char> const & isTransient() const
      { return _isTransient; }

      std::vector<int> const & initAssignment() const
      { return _initAssignment; }

      std::vector<integer> const & resourcesLoadCostWeight() const
      {
         return _resourcesLoadCostWeight;
      }
   

   private:
      std::vector<Resource> _resources;
      std::vector<Machine> _machines;
      std::vector<Service> _services;
      std::vector<Process> _processes;
      std::vector<BalanceCost> _balanceCosts;
      std::vector<Location> _locations;
      std::vector<Neighborhood> _neighborhoods;
      std::vector<unsigned char> _isTransient;
      std::vector<std::pair<int, int> > _dependencies; // "first"
                                                       // depends on
                                                       // "second"

      std::vector<int> _initAssignment;
      int _processMoveCostWeight;
      int _serviceMoveCostWeight;
      int _machineMoveCostWeight;
      int _numNeighborhoods;
      int _numLocations;
      int _numResources;
      std::vector<integer> _resourcesLoadCostWeight;
   };
}
#endif
