#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include "binary_heap.hpp"
#include "instance.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>
#include <set>
#include <vector>

namespace sol
{
   typedef inst::integer integer;

   struct State
   {
      State(inst::Instance const * inst,
            std::vector<int> const & assignment)
         : inst(inst),
           assignment(assignment)
      {
      }

      inst::Instance const * inst;
      std::vector<int> assignment;
   };

   class ObjValue
   {
   public:
      ObjValue()
         : _load(0),
           _balance(0),
           _processMove(0),
           _serviceMove(0),
           _machineMove(0),
           _objValue(0)
      {
      }

      ObjValue(integer load, integer balance, integer processMove,
               integer serviceMove, integer machineMove)
         : _load(load),
           _balance(balance),
           _processMove(processMove),
           _serviceMove(serviceMove),
           _machineMove(machineMove),
           _objValue(_load + _balance + _processMove + _serviceMove
                     + _machineMove)
      {
      }

      void applyDelta(ObjValue const & delta)
      {
         _load += delta._load;
         _balance += delta._balance;
         _processMove += delta._processMove;
         _serviceMove += delta._serviceMove;
         _machineMove += delta._machineMove;
         _objValue += delta._objValue;
      }

      integer load() const { return _load; }
      integer balance() const { return _balance; }
      integer processMove() const { return _processMove; }
      integer serviceMove() const { return _serviceMove; }
      integer machineMove() const { return _machineMove; }
      
      integer objValue() const { return _objValue; }

      void printAll(std::ostream & out) const
      {
         out << "(" << load() 
             << "; " << balance()
             << "; " << processMove()
             << "; " << serviceMove()
             << "; " << machineMove()
             << ")";
      }

      bool operator==(ObjValue const & other) const
      {
      	 return _load == other._load
      	    && _balance == other._balance
      	    && _processMove == other._processMove
      	    && _serviceMove == other._serviceMove
      	    && _machineMove == other._machineMove
	    && _objValue == other._objValue;
      }

      bool operator<(ObjValue const & other) const
      {
         return _objValue < other._objValue;
      }

   private:
      integer _load;
      integer _balance;
      integer _processMove;
      integer _serviceMove;
      integer _machineMove;

      integer _objValue;
   };


   class MachineUsage
   {
   public:
      MachineUsage(State const & state)
      : _machineUsage(
         state.inst->numMachines(),
         std::vector<integer>(state.inst->numResources(), 0)),

        _machineUsageTransient(
           state.inst->numMachines(),
           std::vector<integer>(state.inst->numResources(), 0)),

        _overSafetyCapacity(
           state.inst->numMachines(),
           std::vector<integer>(state.inst->numResources(), 0)),

        _underSafetyCapacity(
           state.inst->numMachines(),
           std::vector<integer>(state.inst->numResources(), 0))

      {
         for (int i = 0; i < state.assignment.size(); i++)
         {
            int process = i;
            int machine = state.assignment[i];

            for (int j = 0; j < state.inst->numResources(); j++)
            {
               int resource = j;
               integer requirement
                  = state.inst->process(process).requirement(resource);
               
               _machineUsage[machine][resource] += requirement;
               _machineUsageTransient[machine][resource] += requirement;
            }
         }

         for (int i = 0; i < state.inst->numMachines(); i++)
         {

            for (int j = 0; j < state.inst->numResources(); j++)
            {
               integer safetyCapacity
                  = state.inst->machine(i).safetyCapacity(j);

               _overSafetyCapacity[i][j] = _machineUsage[i][j] - safetyCapacity;

               if (_overSafetyCapacity[i][j] <= 0)
                  _underSafetyCapacity[i][j] = -_overSafetyCapacity[i][j];
            }
         }
      }

      void moveProcess(State const & state, int process, int srcMachine,
                       int dstMachine)
      {
         for (int i = 0; i < state.inst->numResources(); i++)
         {
            integer requirement = state.inst->process(process).requirement(i);
            
            _machineUsage[srcMachine][i] -= requirement;
            _machineUsage[dstMachine][i] += requirement;

            _overSafetyCapacity[srcMachine][i] -= requirement;
            _overSafetyCapacity[dstMachine][i] += requirement;

            _underSafetyCapacity[srcMachine][i] 
               = std::max(static_cast<integer>(0),
                          -_overSafetyCapacity[srcMachine][i]);

            _underSafetyCapacity[dstMachine][i] 
               = std::max(static_cast<integer>(0),
                          -_overSafetyCapacity[dstMachine][i]);

            if (state.inst->resource(i).transient())
            {
               bool initialSrcMachine = srcMachine
                  == state.inst->initAssignment()[process];

               bool initialDstMachine = dstMachine
                  == state.inst->initAssignment()[process];

               if (!initialSrcMachine)
               {
                  _machineUsageTransient[srcMachine][i] -= requirement;
               }

               if (!initialDstMachine)
               {
                  _machineUsageTransient[dstMachine][i] += requirement;
               }
            }
            else
            {
               _machineUsageTransient[srcMachine][i] -= requirement;
               _machineUsageTransient[dstMachine][i] += requirement;
            }
         }
      }

      std::vector<std::vector<integer> > const & usages() const
      { return _machineUsage; }

      std::vector<integer> const & usage(int machine) const
      {
         return _machineUsage[machine];
      }

      std::vector<integer> const & usageWithTransient(int machine) const
      {
         return _machineUsageTransient[machine];
      }

      std::vector<integer> const & overSafetyCapacity(int machine) const
      {
         return _overSafetyCapacity[machine];
      }

      std::vector<integer> const & underSafetyCapacity(int machine) const
      {
         return _underSafetyCapacity[machine];
      }

      
   private:
      // machine -> resource
      std::vector<std::vector<integer> > _machineUsage;

      // machine -> resource
      std::vector<std::vector<integer> > _machineUsageTransient; 

      // machine -> over safety capacity (can be negative)
      std::vector<std::vector<integer> > _overSafetyCapacity; 

      // machine -> under safety capacity (non negative, under =
      // max(0, -over))
      std::vector<std::vector<integer> > _underSafetyCapacity; 
   };

   class LoadCost
   {
   public:

      integer computeObjValue(
         State const & state,
         std::vector<std::vector<integer> > const & machinesUsage)
      {
         integer objValue = 0;
         
         for (int i = 0; i < state.inst->numResources(); i++)
         {
            integer resourceObjValue = 0;
            
            int resource = i;
            for (int j = 0; j < state.inst->numMachines(); j++)
            {
               int machine = j;

               integer capacity
                  = state.inst->machine(machine).capacity(resource);
               integer usage    = machinesUsage[machine][resource];
               integer safetyCap
                  = state.inst->machine(machine).safetyCapacity(resource);
               
               resourceObjValue
                  += std::max(static_cast<integer>(0),
                              std::min(capacity, usage) - safetyCap);
            }

            resourceObjValue *= state.inst->resource(resource).loadCostWeight();

            objValue += resourceObjValue;
         }

         return objValue;
      }

      integer evaluateMoveProcess(
         State const & state, int process,
         int srcMachine, int dstMachine,
         std::vector<integer> const & srcMachineUsage,
         std::vector<integer> const & dstMachineUsage,
         std::vector<integer> const & srcMachineOverSafetyCapacity,
         std::vector<integer> const & dstMachineUnderSafetyCapacity) const
      {
         integer deltaObjValue = 0;

         std::vector<integer> const & requirements 
            = state.inst->process(process).requirements();

         std::vector<integer> const & resourcesLoadCostWeight 
            = state.inst->resourcesLoadCostWeight();

         int numResources = state.inst->numResources();

         for (int i = 0; i < numResources; i++)
         {
            // Common
            integer loadCostWeight = resourcesLoadCostWeight[i];
            integer processRequirement = requirements[i];

            {
               // Source
               integer overSafetyCapacity = srcMachineOverSafetyCapacity[i];
               
               if (overSafetyCapacity > 0)
               {
                  integer deltaSafetyCapacity
                     = std::min(overSafetyCapacity, processRequirement);
                  
                  deltaObjValue -= loadCostWeight * deltaSafetyCapacity;
               }
            }

            {
               // Destination            
               integer underSafetyCapacity = dstMachineUnderSafetyCapacity[i];
               
               integer deltaSafetyCapacity 
                  = std::max(static_cast<integer>(0),
                             processRequirement - underSafetyCapacity);
               
               if (deltaSafetyCapacity)
                  deltaObjValue += loadCostWeight * deltaSafetyCapacity;
            }
         }

         return deltaObjValue;
      }
   };

   class Balance
   {
   public:
      Balance()
      {
      }

      integer computeObjValue(
         State const & state,
         std::vector<std::vector<integer> > const & machinesUsage)
      {
         integer objValue = 0;
         
         for (int i = 0; i < state.inst->numBalanceCosts(); i++)
         {
            integer balanceObjValue = 0;
            
            inst::BalanceCost const & balanceCost = state.inst->balanceCost(i);
            
            for (int j = 0; j < state.inst->numMachines(); j++)
            {
               int machine = j;

               integer capacityFirstRes
                  = state.inst->machine(machine).capacity(
                     balanceCost.firstResource());
               
               integer usageFirstRes
                  = machinesUsage[machine][balanceCost.firstResource()];

               integer capacitySecondRes
                  = state.inst->machine(machine).capacity(
                     balanceCost.secondResource());
               
               integer usageSecondRes
                  = machinesUsage[machine][balanceCost.secondResource()];


               balanceObjValue
                  += std::max(static_cast<integer>(0),
                              balanceCost.target() *
                              (capacityFirstRes -
                               std::min(capacityFirstRes, usageFirstRes))
                              - (capacitySecondRes -
                                 std::min(capacitySecondRes, usageSecondRes)));
            }

            balanceObjValue *= balanceCost.weight();

            objValue += balanceObjValue;
         }

         return objValue;
      }

      integer evaluateMoveProcess(
         State const & state,
         int process,
         int srcMachine,
         int dstMachine,
         std::vector<integer> const & srcMachineUsage,
         std::vector<integer> const & dstMachineUsage) const
      {
         integer deltaObjValue = 0;

         for (int i = 0; i < state.inst->numBalanceCosts(); i++)
         {
            integer delta = 0;
            inst::BalanceCost const & balanceCost = state.inst->balanceCost(i);

            int firstRes = balanceCost.firstResource();
            int secondRes = balanceCost.secondResource();
            
            // Source            
                         
            integer deltaFirstResSrc = deltaResourceRemove(
               state,
               process,
               srcMachine,
               balanceCost.firstResource(),
               srcMachineUsage);

            integer deltaSecondResSrc = deltaResourceRemove(
               state,
               process,
               srcMachine,
               balanceCost.secondResource(),
               srcMachineUsage);

            integer remainingFirstResBeforeSrc
               = std::max(static_cast<inst::integer>(0),
                          state.inst->machine(srcMachine)
                          .capacity(firstRes) - srcMachineUsage[firstRes]);
            
            integer remainingSecondResBeforeSrc
               = std::max(static_cast<inst::integer>(0),
                          state.inst->machine(srcMachine)
                          .capacity(secondRes)
                          - srcMachineUsage[secondRes]);

            integer remainingFirstResAfterSrc
               = remainingFirstResBeforeSrc - deltaFirstResSrc;

            integer remainingSecondResAfterSrc
               = remainingSecondResBeforeSrc - deltaSecondResSrc;

            
            
            integer valueBeforeSrc = 
               std::max(static_cast<inst::integer>(0),
                          balanceCost.target() * remainingFirstResBeforeSrc
                          - remainingSecondResBeforeSrc);

            integer valueAfterSrc = 
               std::max(static_cast<inst::integer>(0),
                          balanceCost.target() * remainingFirstResAfterSrc
                          - remainingSecondResAfterSrc);

            
            delta += valueAfterSrc - valueBeforeSrc;


            // Destination

            integer deltaFirstResDst = deltaResourceAdd(
               state,
               process,
               dstMachine,
               balanceCost.firstResource(),
               dstMachineUsage);

            integer deltaSecondResDst = deltaResourceAdd(
               state,
               process,
               dstMachine,
               balanceCost.secondResource(),
               dstMachineUsage);


            integer remainingFirstResBeforeDst
               = std::max(static_cast<inst::integer>(0),
                          state.inst->machine(dstMachine)
                          .capacity(firstRes) - dstMachineUsage[firstRes]);
            
            integer remainingSecondResBeforeDst
               = std::max(static_cast<inst::integer>(0),
                          state.inst->machine(dstMachine)
                          .capacity(secondRes)
                          - dstMachineUsage[secondRes]);

            integer remainingFirstResAfterDst
               = remainingFirstResBeforeDst - deltaFirstResDst;

            integer remainingSecondResAfterDst
               = remainingSecondResBeforeDst - deltaSecondResDst;
            
            integer valueBeforeDst = /* balanceCost.weight()
                                      * */
               std::max(static_cast<inst::integer>(0),
                          balanceCost.target() * remainingFirstResBeforeDst
                          - remainingSecondResBeforeDst);

            integer valueAfterDst = 
               std::max(static_cast<inst::integer>(0),
                          balanceCost.target() * remainingFirstResAfterDst
                          - remainingSecondResAfterDst);


            delta += valueAfterDst - valueBeforeDst;

            deltaObjValue += balanceCost.weight() * delta;
         }

         return deltaObjValue;
      }

   private:
      
      integer deltaResourceRemove(
         State const & state,
         int process,
         int machine,
         int resource,
         std::vector<integer> const & machineUsage) const
      {
         integer overUsage = std::max(
            static_cast<integer>(0),
            machineUsage[resource] -
            state.inst->machine(machine).capacity(resource));


         integer requirement
            = state.inst->process(process).requirement(resource);
         
         integer delta
            = std::min(
	       static_cast<integer>(0),
	       overUsage - requirement);

         return delta;
      }

      integer deltaResourceAdd(
         State const & state,
         int process,
         int machine,
         int resource,
         std::vector<integer> const & machineUsage) const
      {
         integer underUsage = std::max(
            static_cast<integer>(0),
            state.inst->machine(machine).capacity(resource)
            - machineUsage[resource]);

         

         integer requirement
            = state.inst->process(process).requirement(resource);

         integer delta = std::min(
            underUsage,
            requirement);



         return delta;
      }
   };

   class ProcessMove
   {
   public:

      integer computeObjValue(State const & state)
      {
         integer objValue = 0;
         
         for (int i = 0; i < state.inst->numProcesses(); i++)
         {
            if (state.assignment[i] != state.inst->initAssignment()[i])
            {
               objValue += state.inst->process(i).moveCost();
            }
         }

         objValue *= state.inst->processMoveCostWeight();

         return objValue;
      }
      
      integer evaluateMoveProcess(
         State const & state,
         int process,
         int srcMachine,
         int dstMachine) const
      {
         integer deltaObjValue = 0;

         int initMachine = state.inst->initAssignment()[process];

         if (srcMachine == initMachine)
         {
            deltaObjValue += state.inst->process(process).moveCost();
         }
         else if (dstMachine == initMachine)
         {
            deltaObjValue -= state.inst->process(process).moveCost();
         }

         deltaObjValue *= state.inst->processMoveCostWeight();

         return deltaObjValue;
      }
   };

   class ServiceMove
   {
      class ServiceNode
      {
      public:
         ServiceNode() : _numProcMoved(0), _heapPosition(-1)
         {
         }

         int numProcMoved() const { return _numProcMoved; }
         
         void decNumProMoved() { _numProcMoved--; }
         void incNumProMoved() { _numProcMoved++; }

         int heapPosition() const { return _heapPosition; }
         
         void setHeapPosition(int heapPosition)
         { _heapPosition = heapPosition; }
         
      private:
         int _numProcMoved;
         int _heapPosition;
      };

      class CompareServiceNode
      {
      public:
         
         CompareServiceNode(std::vector<ServiceNode> const & services)
            : _services(services)
         {
         }
            
         bool operator()(int a, int b)
         {
            return _services[a].numProcMoved() < _services[b].numProcMoved();
         }
         
      private:
         std::vector<ServiceNode> const & _services;
      };

      class UpdateServiceNodePos
      {
      public:
         UpdateServiceNodePos(std::vector<ServiceNode> & services)
            : _services(services)
         {
         }

         void operator()(int service, int heapPosition)
         {
            _services[service].setHeapPosition(heapPosition);
         }
         
      private:
         std::vector<ServiceNode> & _services;         
      };
      
   public:
      ServiceMove(State const & state)
         : _firstChild(std::min(1, state.inst->numServices() - 1)),
           _secondChild(std::min(2, state.inst->numServices() - 1))
      {
         for (int i = 0; i < state.inst->numServices(); i++)
         {
            _services.push_back(ServiceNode());
            _services[i].setHeapPosition(i);
            _heap.push_back(i);
         }
      }

      integer computeObjValue(State const & state)
      {
         std::vector<int> numProcPerService(state.inst->numServices(), 0);
         
         for (int i = 0; i < state.assignment.size(); i++)
         {
            if (state.assignment[i] != state.inst->initAssignment()[i])
            {
               int service = state.inst->process(i).service();

               numProcPerService[service]++;
            }
         }

         int max = *std::max_element(numProcPerService.begin(),
                                     numProcPerService.end());

         return max * state.inst->serviceMoveCostWeight();
      }

      integer evaluateMoveProcess(
         State const & state,
         int process,
         int srcMachine,
         int dstMachine) const
      {
         
         integer deltaObjValue = 0;

         int service = state.inst->process(process).service();
         int initMachine = state.inst->initAssignment()[process];


         int numProcMoved = _services[service].numProcMoved();
         int bestNumProcMoved = _services[_heap[0]].numProcMoved();

         if (srcMachine == initMachine)
         {
            // Increment

            if (numProcMoved == bestNumProcMoved)
            {
               deltaObjValue += state.inst->serviceMoveCostWeight();
            }
         }
         else if (dstMachine == initMachine)
         {
            // Decrement

            if (numProcMoved == bestNumProcMoved)
            {
               if (_services[_heap[_firstChild]].numProcMoved()
                   < bestNumProcMoved &&
                   _services[_heap[_secondChild]].numProcMoved()
                   < bestNumProcMoved)
               {
                  deltaObjValue -= state.inst->serviceMoveCostWeight();
               }
            }
         }

         return deltaObjValue;         
      }


      void moveProcess(State const & state,
                       int process,
                       int srcMachine,
                       int dstMachine)
      {
         int service = state.inst->process(process).service();
         int initMachine = state.inst->initAssignment()[process];

         if (srcMachine == initMachine)
         {
            
            _services[service].incNumProMoved();

            
            update_heap_pos(_heap.begin(), _heap.end(),
                            _heap.begin() + _services[service].heapPosition(),
                            CompareServiceNode(_services),
                            UpdateServiceNodePos(_services));

            

         }
         else if (dstMachine == initMachine)
         {
            
            _services[service].decNumProMoved();
            update_heap_pos(_heap.begin(), _heap.end(),
                            _heap.begin() + _services[service].heapPosition(),
                            CompareServiceNode(_services),
                            UpdateServiceNodePos(_services));
            
         }
      }


   private:

      std::vector<ServiceNode> _services;
      std::vector<int> _heap;

      int _firstChild;
      int _secondChild;
   };

   class MachineMove
   {
   public:

      integer computeObjValue(State const & state)
      {
         integer objValue = 0;

         for (int i = 0; i < state.inst->numProcesses(); i++)
         {
            objValue += state.inst->machine(state.inst->initAssignment()[i])
               .moveCost(state.assignment[i]);
         }

         objValue *= state.inst->machineMoveCostWeight();

         return objValue;
      }
      
      
      integer evaluateMoveProcess(State const & state, int process,
                                  int srcMachine, int dstMachine)
      {
         integer deltaObjValue = 0;

         
         int initMachine = state.inst->initAssignment()[process];
	 

         if (srcMachine == initMachine)
         {
            deltaObjValue += state.inst->machine(initMachine).
               moveCost(dstMachine);
         }
         else if (dstMachine == initMachine)
         {
            deltaObjValue -= state.inst->machine(initMachine).
               moveCost(srcMachine);
         }
         else
         {
            deltaObjValue
               += -state.inst->machine(initMachine).moveCost(srcMachine)
               + state.inst->machine(initMachine).moveCost(dstMachine);
         }


         deltaObjValue *= state.inst->machineMoveCostWeight();

         return deltaObjValue;
      }
   };


   // This class handles "transient" constraints.
   class Capacity
   {
   public:
      Capacity(State const & state)
      {
         
      }

      bool isFeasible(State const & state, int process,
                      int srcMachine, int dstMachine,
                      std::vector<integer> const & srcMachineUsage,
                      std::vector<integer> const & dstMachineUsage,
                      std::vector<integer> const & requirements,
                      std::vector<integer> const & srcCapacities,
                      std::vector<integer> const & dstCapacities)
      {
         std::vector<unsigned char> const & isTransient
            = state.inst->isTransient();
         
         std::vector<int> const & initAssignment
            = state.inst->initAssignment();
         
         bool isInitialDstMachine
            = (dstMachine == initAssignment[process]);
         
         int numResources = state.inst->numResources();
         for (int i = 0; i < numResources; i++)
         {
            if (!(isTransient[i] && isInitialDstMachine))
            {
               if ((dstMachineUsage[i] + requirements[i]) > dstCapacities[i])
                  return false;
            }
         }
         return true;
      }

      // Capacity::moveProcess doesn't exist because Capacity doesn't
      // have any state.
   };

   class Conflict
   {
   public:
      Conflict(State const & state)
         : _servMachNumProc(state.inst->numServices(),
                            std::vector<int>(state.inst->numMachines(), 0))
      {
         for (int i = 0; i < state.assignment.size(); i++)
         {
            int service = state.inst->process(i).service();
            int machine = state.assignment[i];

            _servMachNumProc[service][machine]++;
         }
      }

      bool isFeasible(State const & state, int process,
                      int srcMachine, int dstMachine, int service)
      {
         return _servMachNumProc[service][dstMachine] == 0;
      }

      void moveProcess(State const & state, int process, int srcMachine,
                       int dstMachine)
      {
         int service = state.inst->process(process).service();

         _servMachNumProc[service][srcMachine]--;
         _servMachNumProc[service][dstMachine]++;
      }

   private:
      std::vector<std::vector<int> > _servMachNumProc; // service -> machine
   };

   class Spread
   {
   public:
      Spread(State const & state)
         : _servLocNumProc(state.inst->numServices(),
                           std::vector<int>(state.inst->numLocations(), 0)),
           _servNumLoc(state.inst->numServices(), 0)
      {
         for (int i = 0; i < state.assignment.size(); i++)
         {
            int service = state.inst->process(i).service();
            int machine = state.assignment[i];
            int location = state.inst->machine(machine).location();

            _servLocNumProc[service][location]++;
         }

         for (int i = 0; i < state.inst->numServices(); i++)
         {
            for (int j = 0; j < state.inst->numLocations(); j++)
            {
               if (_servLocNumProc[i][j] >= 1)
               {
                  _servNumLoc[i]++;
               }
            }
         }
      }


      bool isFeasible(State const & state, int process,
                      int srcMachine,
                      int dstMachine, int service,
                      int srcLocation,
                      int dstLocation)
      {
         if (srcLocation == dstLocation)
            return true;

         bool srcLocIsEmpty = _servLocNumProc[service][srcLocation] == 1;
         bool dstLocIsEmpty = _servLocNumProc[service][dstLocation] == 0;

         integer spreadMin = state.inst->service(service).spreadMin();
         
         if (srcLocIsEmpty && !dstLocIsEmpty)
         {
            // Service will lost one location, check if it has the
            // minimum requirement of locations.
            return _servNumLoc[service] - 1 >= spreadMin;
         }

         return true;
      }


      void moveProcess(State const & state, int process, int srcMachine,
                       int dstMachine)
      {
         int service = state.inst->process(process).service();
         int srcLocation = state.inst->machine(srcMachine).location();
         int dstLocation = state.inst->machine(dstMachine).location();

         if (srcLocation != dstLocation)
         {
            _servLocNumProc[service][srcLocation]--;

            if (_servLocNumProc[service][srcLocation] == 0)
               _servNumLoc[service]--;

            _servLocNumProc[service][dstLocation]++;

            if (_servLocNumProc[service][dstLocation] == 1)
               _servNumLoc[service]++;
         }
      }
      
   private:
      std::vector<std::vector<int> > _servLocNumProc; // service -> location
      std::vector<int> _servNumLoc; // service
   };

   class Dependency
   {
   public:
      Dependency(State const & state)
         : _servNeighNumProc(state.inst->numServices(),
                             std::vector<int>(state.inst->numNeighborhoods(),
                                              0))
      {
         for (int i = 0; i < state.assignment.size(); i++)
         {
            int process = i;
            int machine = state.assignment[i];
            int service = state.inst->process(process).service();
            int neighborhood = state.inst->machine(machine).neighborhood();

            _servNeighNumProc[service][neighborhood]++;
         }
      }
      
      bool isFeasible(State const & state, int process,
                                  int srcMachine,
                                  int dstMachine, int service,
                                  int srcNeighborhood,
                                  int dstNeighborhood)
      {
         if (srcNeighborhood == dstNeighborhood)
            return true;

         // Source neighborhood
         
         bool lastProcessInNeigh
            = _servNeighNumProc[service][srcNeighborhood] == 1;

         if (lastProcessInNeigh)
         {
            std::vector<int> const & reverseDependencies
               = state.inst->service(service).reverseDependencies();

            for (int i = 0; i < reverseDependencies.size(); i++)
            {
               int otherService = reverseDependencies[i];
               
               if (_servNeighNumProc[otherService][srcNeighborhood] >= 1)
               {
                  return false;
               }
            }
         }

         // Destination neighborhood
         
         bool  firstProcessInNeigh
            = _servNeighNumProc[service][dstNeighborhood] == 0;

         if (firstProcessInNeigh)
         {
            std::vector<int> const & dependencies
               = state.inst->service(service).dependencies();

            for (int i = 0; i < dependencies.size(); i++)
            {
               int otherService = dependencies[i];
               
               if (_servNeighNumProc[otherService][dstNeighborhood] == 0)
               {
                  return false;
               }
            }
         }

         return true;
      }

      void moveProcess(State const & state, int process, int srcMachine,
                       int dstMachine)
      {
         int service = state.inst->process(process).service();
         int srcNeighborhood = state.inst->machine(srcMachine).neighborhood();
         int dstNeighborhood = state.inst->machine(dstMachine).neighborhood();

         if (srcNeighborhood == dstNeighborhood)
            return;

         _servNeighNumProc[service][srcNeighborhood]--;
         _servNeighNumProc[service][dstNeighborhood]++;
      }


   private:

      std::vector<std::vector<int> > _servNeighNumProc; // service -> location
      
   };
   
   class Solution 
   {
   public:
      
      Solution(inst::Instance const * instance)
         : _state(instance, instance->initAssignment()),
           _machineUsage(_state),
           _serviceMove(_state),
           _conflict(_state),
           _spread(_state),
           _capacity(_state),
           _dependency(_state)
      {
      }

      std::vector<int> const & assignment() const { return _state.assignment; }

      // ** SLOW ** It computes from scratch the objective value. It
      // should be used once at the beginning of the algorithm or for
      // debugging purpose.
      ObjValue computeObjValue()
      {
         integer loadObjValue
            = _loadCost.computeObjValue(_state, _machineUsage.usages());

         integer balanceObjValue
            = _balance.computeObjValue(_state, _machineUsage.usages());

         integer processMoveObjValue =
            _processMove.computeObjValue(_state);

         integer serviceMoveObjValue =
            _serviceMove.computeObjValue(_state);

         integer machineMoveObjValue =
            _machineMove.computeObjValue(_state);

         return ObjValue(loadObjValue, balanceObjValue,
                         processMoveObjValue, serviceMoveObjValue,
                         machineMoveObjValue);
      }

      void applyDelta(ObjValue const & deltaObjValue)
      {
         _objValue.applyDelta(deltaObjValue);
      }

      ObjValue evaluateFeasibleMove(int process, int dstMachine)
      {
         int srcMachine = _state.assignment[process];

         if (srcMachine == dstMachine)
            return ObjValue();

         std::vector<integer> const & srcMachineUsage
            = _machineUsage.usage(srcMachine);
         
         std::vector<integer> const & dstMachineUsage
            = _machineUsage.usage(dstMachine);

         std::vector<integer> const & srcMachineOverSafetyCapacity
            = _machineUsage.overSafetyCapacity(srcMachine);
         
         std::vector<integer> const & dstMachineUnderSafetyCapacity
            = _machineUsage.underSafetyCapacity(dstMachine);

         integer deltaObjValueLoad = _loadCost.evaluateMoveProcess(
            _state, process, srcMachine, dstMachine, srcMachineUsage,
            dstMachineUsage, srcMachineOverSafetyCapacity, 
            dstMachineUnderSafetyCapacity);

         integer deltaObjValueBalance = _balance.evaluateMoveProcess(
            _state, process, srcMachine, dstMachine, srcMachineUsage,
            dstMachineUsage);

         integer deltaObjValueProcessMove = _processMove.evaluateMoveProcess(
            _state, process, srcMachine, dstMachine);

         integer deltaObjValueServiceMove = _serviceMove.evaluateMoveProcess(
            _state, process, srcMachine, dstMachine);

         integer deltaObjValueMachineMove = _machineMove.evaluateMoveProcess(
            _state, process, srcMachine, dstMachine);

         return ObjValue(
            deltaObjValueLoad,
            deltaObjValueBalance,
            deltaObjValueProcessMove,
            deltaObjValueServiceMove,
            deltaObjValueMachineMove);
      }

      // It assumes that the current solution is feasible.
      bool isFeasible(int process, int dstMachine)
      {
         int srcMachine = _state.assignment[process];
         
         if (srcMachine == dstMachine)
            return true;

         std::vector<integer> const & srcMachineUsageTransient
            = _machineUsage.usageWithTransient(srcMachine);
         
         std::vector<integer> const & dstMachineUsageTransient
            = _machineUsage.usageWithTransient(dstMachine);

         inst::Process const & processObj = _state.inst->process(process);
         int service = processObj.service();

         inst::Machine const & machineSrcObj = _state.inst->machine(srcMachine);
         inst::Machine const & machineDstObj = _state.inst->machine(dstMachine);


         bool spreadFeasible
            = _spread.isFeasible(_state,
                                 process,
                                 srcMachine,
                                 dstMachine,
                                 service,
                                 machineSrcObj.location(),
                                 machineDstObj.location());
         if (!spreadFeasible)
            return false;

         bool dependencyFeasible
            = _dependency.isFeasible(_state,
                                     process,
                                     srcMachine,
                                     dstMachine,
                                     service,
                                     machineSrcObj.neighborhood(),
                                     machineDstObj.neighborhood());
         if (!dependencyFeasible)
            return false;

         bool conflictFeasible
            = _conflict.isFeasible(_state, process,
                                   srcMachine,
                                   dstMachine,
                                   service);
         if (!conflictFeasible)
            return false;


         bool capacityFeasible = _capacity.isFeasible(
            _state, process, srcMachine, dstMachine,
            srcMachineUsageTransient,
            dstMachineUsageTransient,
            processObj.requirements(),
            machineSrcObj.capacities(),
            machineDstObj.capacities());

         if (!capacityFeasible)
            return false;

         return true;
      }

      void moveProcess(int process, int dstMachine,
                       ObjValue const & deltaObjValue)
      {
         int srcMachine = _state.assignment[process];

         if (srcMachine == dstMachine)
            return;

         _machineUsage.moveProcess(_state, process, srcMachine, dstMachine);
         
         std::vector<integer> const & newSrcMachineUsageTransient
            = _machineUsage.usageWithTransient(srcMachine);
         
         std::vector<integer> const & newDstMachineUsageTransient
            = _machineUsage.usageWithTransient(dstMachine);

         
         _serviceMove.moveProcess(_state, process, srcMachine, dstMachine);
         // Capacity::moveProcess doesn't exist because Capacity
         // doesn't have any state.
         _conflict.moveProcess(_state, process, srcMachine, dstMachine);
         _spread.moveProcess(_state, process, srcMachine, dstMachine);
         _dependency.moveProcess(_state, process, srcMachine, dstMachine);

         _state.assignment[process] = dstMachine;

         _objValue.applyDelta(deltaObjValue);


         inst::Machine const & machineDstObj = _state.inst->machine(dstMachine);

         for (int i = 0; i <  newDstMachineUsageTransient.size(); i++)
         {
            if (newDstMachineUsageTransient[i] > machineDstObj.capacity(i))
            {
               std::cout << "violation" << std::endl;
               std::cout << "Machine: " << dstMachine << std::endl;
               std::cout << "Resource: " << i << std::endl;
               std::cout << "Usage: " << newDstMachineUsageTransient[i]
                         << std::endl;
               std::cout << "Capacity: " << machineDstObj.capacity(i)
                         << std::endl;
               throw -1;
            }
         }
      }

      ObjValue const & objValue() const
      {
         return _objValue;
      }


   private:
      State _state; 
      
      MachineUsage _machineUsage; 

      LoadCost _loadCost;       // No attribute
      Balance _balance;         // No attribute
      ProcessMove _processMove; // No attribute
      ServiceMove _serviceMove;
      MachineMove _machineMove; // No attribute

      Capacity _capacity;       // No attribute
      Conflict _conflict;
      Spread _spread;
      Dependency _dependency;

      ObjValue _objValue;
   };
}

std::ostream& operator<<(std::ostream & out,
                         sol::ObjValue const & objValue);


#endif
