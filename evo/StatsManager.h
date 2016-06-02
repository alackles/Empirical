#ifndef EMP_EVO_STATS_MANAGER_H
#define EMP_EVO_STATS_MANAGER_H

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../tools/FunctionSet.h"
#include "../tools/vector.h"
#include "../tools/stats.h"
#include "../config/config.h"
#include "PopulationManager.h"

namespace emp{
namespace evo{

  EMP_BUILD_CONFIG( StatsManagerConfig,
    VALUE(RESOLUTION, int, 10, "How often should stats be calculated (updates)"),
    VALUE(DELIMITER, std::string, " ", "What should fields be separated by in the output")
  )

  //Base stats manager - this mostly exists to be extended into custom
  //stats managers (see the OEEStatsManager for an example). The base
  //stats manager also handles data output.
  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_Base {
  public:
    std::string delimiter = " "; //Gets inferred from file name
    int resolution = 10; //With what frequency do we record data?
    static constexpr bool emp_is_stats_manager = true;
    std::ofstream output_location; //Where does output go?

    StatsManager_Base(std::string location = "cout"){
        StatsManagerConfig config;
        config.Read("StatsConfig.cfg");
        resolution = config.RESOLUTION();
        delimiter = config.DELIMITER();
        config.Write("StatsConfig.cfg");
        SetOutput(location);
    }

    StatsManager_Base(StatsManagerConfig & config, std::string config_location,
                      std::string location = "cout"){
        config.Read(config_location);
        resolution = config.RESOLUTION();
        delimiter = config.DELIMITER();
        SetOutput(location);
    }

    ~StatsManager_Base(){
        output_location.close();
    }

    template <typename WORLD>
    void Setup(WORLD * w){;}

    template <typename T>
    void SetDefaultFitnessFun(std::function<double(T)> fit){;}

    //Tells the stats manager where to put output. If location is "cout"
    //(default) or "stdout", stats will get sent to cout. Otherwise, the
    //specified file will be used as the location for output. If the file
    //has the extension "csv" or "tsv", the appropriate delimiter will be used.
    //If the location is invalid, the program will exit with an error.
    void SetOutput(std::string location){
        if (location == "cout" || location == "stdout"){
            output_location.copyfmt(std::cout);
            output_location.clear(std::cout.rdstate());
            output_location.basic_ios<char>::rdbuf(std::cout.rdbuf());
        } else {
            output_location.open(location);
            if (!output_location.good()){
                std::cout << "Invalid output file. Exiting." << std::endl;
                exit(0);
            }
            string_pop(location, ".");
            if (location == "csv") {
                delimiter = ", ";
            } else if (location == "tsv") {
                delimiter = "\t";
            }
        }
    }

  };

  //A popular type of stats manager is one that prints a set of statistics every
  //so many updates. This is a generic stats manager of that variety, which
  //maintains FunctionSets containing all of the functions to be run.
  //Although functions can be added to this manager on the fly, the goal of
  //this class is that it can be extended to track specific sets of functions.
  //(see StatsManager_DefaultStats for an example)
  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_FunctionsOnUpdate : StatsManager_Base<POP_MANAGER> {
  protected:
    using org_ptr = typename POP_MANAGER::value_type;
    //using world_type = World<ORG, MANAGERS...>;
    using fit_fun_type = std::function<double(org_ptr)>;
    //Stats calculated on the world
    FunctionSet<double, POP_MANAGER * > world_stats;
    //Stats calculated on the world that require a fitness function
    FunctionSet<double, std::function<double(org_ptr)>,
                                           POP_MANAGER* > fitness_stats;
    //Pointer to the world object on which we're calculating stats
    POP_MANAGER * pop;
    using StatsManager_Base<POP_MANAGER>::resolution;
    using StatsManager_Base<POP_MANAGER>::output_location;
    using StatsManager_Base<POP_MANAGER>::delimiter;
    bool header_printed = false;
    std::string header = "update";

  public:
    using StatsManager_Base<POP_MANAGER>::emp_is_stats_manager;
    fit_fun_type fit_fun;

    //Constructor for creating this as a stand-alone object
    template <typename WORLD>
    StatsManager_FunctionsOnUpdate(WORLD * w,
                                   std::string location = "stats.csv") :
                                   StatsManager_Base<decltype(w->popM)>(location){
      Setup(w);
    }

    //Constructor for use by World object
    StatsManager_FunctionsOnUpdate(std::string location = "stats.csv") :
                                   StatsManager_Base<POP_MANAGER>(location){;}

    //The fitness function for calculating fitness related stats
    template <typename WORLD>
    void Setup(WORLD * w){
      pop = &(w->PopM);

      std::function<void(int)> UpdateFun = [&] (int ud){
          Update(ud);
      };

      w->OnUpdate(UpdateFun);
    }

    //Function for adding functions that calculate stats to the
    //set to be calculated
    void AddFunction(std::function<double(POP_MANAGER*)> func, std::string label) {
      world_stats.Add(func);
      if (header_printed){
        NotifyWarning("Function added to stats manager after initialization.");
      } else {
        header += delimiter + label;
      }
    }

    //Version for functions that require a fitness function
    void AddFunction(std::function<double(fit_fun_type, POP_MANAGER*)> func, std::string label) {
      fitness_stats.Add(func);
      if (header_printed){
        NotifyWarning("Function added to stats manager after initialization.");
      } else {
        header += delimiter + label;
      }
    }

    //If this update matches the resolution, calculate and record all the stats
    void Update(int update) {
      if (!header_printed) {
          output_location << header << std::endl;
          header_printed = true;
      }

      if (update % resolution == 0){

        output_location << update;

        emp::vector<double> world_results = world_stats.Run(pop);
        for (double d : world_results) {
          output_location << delimiter << d;
        }

        emp::vector<double> fitness_results = fitness_stats.Run(fit_fun, pop);
        for (double d : fitness_results){
            output_location << delimiter << d;
        }
        output_location << std::endl;
      }
    }

    void SetDefaultFitnessFun(std::function<double(org_ptr)> fit){
        fit_fun = fit;
    }

  };

  //Calculates some commonly required information: shannon diversity,
  //max fitness within the population, and average fitness within the population
  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_DefaultStats : StatsManager_FunctionsOnUpdate<POP_MANAGER> {
  private:
      using org_ptr = typename POP_MANAGER::value_type;
      using fit_fun_type = std::function<double(org_ptr)>;
      using fit_stat_type = std::function<double(fit_fun_type, POP_MANAGER*)>;
      using StatsManager_FunctionsOnUpdate<POP_MANAGER>::AddFunction;
      using StatsManager_FunctionsOnUpdate<POP_MANAGER>::pop;
      using StatsManager_Base<POP_MANAGER>::output_location;
      using StatsManager_FunctionsOnUpdate<POP_MANAGER>::Update;
  public:
      using StatsManager_FunctionsOnUpdate<POP_MANAGER>::fit_fun;
      using StatsManager_Base<POP_MANAGER>::emp_is_stats_manager;
      using StatsManager_FunctionsOnUpdate<POP_MANAGER>::SetDefaultFitnessFun;

      //Constructor for use as a stand-alone object
      template <typename WORLD>
      StatsManager_DefaultStats(WORLD * w, std::string location = "averages.csv")
       : StatsManager_FunctionsOnUpdate<decltype(w->popM)>(w, location){
        Setup(w);
      }

      //Constructor for use as a template parameter for the world
      StatsManager_DefaultStats(std::string location = "averages.csv")
       : StatsManager_FunctionsOnUpdate<POP_MANAGER>(location){;}

      //Add appropriate functions to function sets
      template <typename WORLD>
      void Setup(WORLD * w){
        pop = &(w->popM);

        //Create std::function object for all of the stats
        std::function<double(POP_MANAGER*)> diversity = [](POP_MANAGER * pop){
            return ShannonEntropy(*pop);
        };
        fit_stat_type max_fitness = [](fit_fun_type fit_func, POP_MANAGER * pop){
            return MaxFunctionReturn(fit_func, *pop);
        };
        fit_stat_type avg_fitness = [](fit_fun_type fit_func, POP_MANAGER * pop){
            return AverageFunctionReturn(fit_func, *pop);
        };

        std::function<void(int)> UpdateFun = [&] (int ud){
            Update(ud);
        };

        //Add functions to manager
        AddFunction(diversity, "shannon_diversity");
        AddFunction(max_fitness, "max_fitness");
        AddFunction(avg_fitness, "avg_fitness");

        w->OnUpdate(UpdateFun);
      }

};

using NullStats = StatsManager_Base<PopBasic>;
using DefaultStats = StatsManager_DefaultStats<PopBasic>;
}
}

#endif
