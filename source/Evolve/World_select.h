/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  World_select.h
 *  @brief Functions for popular selection methods applied to worlds.
 */

#ifndef EMP_EVO_WORLD_SELECT_H
#define EMP_EVO_WORLD_SELECT_H

#include <map>
#include <functional>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../base/macros.h"
#include "../tools/IndexMap.h"
#include "../tools/Random.h"
#include "../tools/vector_utils.h"

namespace emp {

  template<typename ORG> class World;

  /// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
  /// the next generation.  Find top e_count individuals and make copy_count copies of each.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param e_count How many distinct organisms should be chosen, starting from the most fit.
  /// @param copy_count How many copies should be made of each elite organism?
  template<typename ORG>
  void EliteSelect(World<ORG> & world, size_t e_count=1, size_t copy_count=1) {
    emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
    emp_assert(copy_count > 0);

    // Load the population into a multimap, sorted by fitness.
    std::multimap<double, size_t> fit_map;
    for (size_t id = 0; id < world.GetSize(); id++) {
      if (world.IsOccupied(id)) {
        const double cur_fit = world.CalcFitnessID(id);
        fit_map.insert( std::make_pair(cur_fit, id) );
      }
    }

    // Grab the top fitnesses and move them into the next generation.
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < e_count; i++) {
      const size_t repro_id = m->second;
      world.DoBirth( world.GetGenomeAt(repro_id), repro_id, copy_count);
      ++m;
    }
  }


  /// ==TOURNAMENT== Selection creates a tournament with a random sub-set of organisms,
  /// finds the one with the highest fitness, and moves it to the next generation.
  /// User provides the world (with a fitness function), the tournament size, and
  /// (optionally) the number of tournaments to run.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param t_size How many organisms should be placed in each tournament?
  /// @param tourny_count How many tournaments should be run? (with replacement of organisms)
  template<typename ORG>
  void TournamentSelect(World<ORG> & world, size_t t_size, size_t tourny_count=1) {
    emp_assert(t_size > 0, t_size, world.GetNumOrgs());
    emp_assert(tourny_count > 0);

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i < t_size; i++) entries.push_back( world.GetRandomOrgID() );

      double best_fit = world.CalcFitnessID(entries[0]);
      size_t best_id = entries[0];

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < t_size; i++) {
        const double cur_fit = world.CalcFitnessID(entries[i]);
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = entries[i];
        }
      }

      // Place the highest fitness into the next generation!
      world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
    }
  }

  /// ==ROULETTE== Selection (aka Fitness-Proportional Selection) chooses organisms to
  /// reproduce based on their current fitness.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param count How many organims should be selected for replication? (with replacement)
  template<typename ORG>
  void RouletteSelect(World<ORG> & world, size_t count=1) {
    emp_assert(count > 0);

    Random & random = world.GetRandom();

    // Load fitnesses from current population.
    IndexMap fitness_index(world.GetSize());
    for (size_t id = 0; id < world.GetSize(); id++) {
      fitness_index.Adjust(id, world.CalcFitnessID(id));
    }

    for (size_t n = 0; n < count; n++) {
      const double fit_pos = random.GetDouble(fitness_index.GetWeight());
      const size_t parent_id = fitness_index.Index(fit_pos);
      const size_t offspring_id = world.DoBirth( world.GetGenomeAt(parent_id), parent_id ).GetIndex();
      if (world.IsSynchronous() == false) {
        fitness_index.Adjust(offspring_id, world.CalcFitnessID(offspring_id));
      }
    }
  }


  EMP_CREATE_OPTIONAL_METHOD(TriggerOnLexicaseSelect, TriggerOnLexicaseSelect);

  /// ==LEXICASE== Selection runs through multiple fitness functions in a random order for
  /// EACH offspring produced.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param fit_funs The set of fitness functions to shuffle for each organism reproduced.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  /// @param max_funs The maximum number of fitness functions to use. (use 0 for all; default)
  template<typename ORG>
  void LexicaseSelect(World<ORG> & world,
                      const emp::vector< std::function<double(const ORG &)> > & fit_funs,
                      size_t repro_count=1,
                      size_t max_funs=0)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(fit_funs.size() > 0);

    // @CAO: Can probably optimize a bit!
    
    std::map<typename ORG::genome_t, int> genotype_counts;
    emp::vector<emp::vector<size_t>> genotype_lists;

    // Find all orgs with same genotype - we can dramatically reduce
    // fitness evaluations this way.
    for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
      if (world.IsOccupied(org_id)) {
        const typename decltype(World<ORG>())::genome_t gen = world.GetGenomeAt(org_id);
        if (emp::Has(genotype_counts, gen)) {
          genotype_lists[genotype_counts[gen]].push_back(org_id);
        } else {
          genotype_counts[gen] = genotype_lists.size();
          genotype_lists.emplace_back(emp::vector<size_t>{org_id});
        }
      }
    }

    emp::vector<size_t> all_gens(genotype_lists.size()), cur_gens, next_gens;

    for (size_t i = 0; i < genotype_lists.size(); i++) {
      all_gens[i] = i;
    }

    if (!max_funs) max_funs = fit_funs.size();
    // std::cout << "in lexicase" << std::endl;
    // Collect all fitness info. (@CAO: Technically only do this is cache is turned on?)
    emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
    for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
      fitnesses[fit_id].resize(genotype_counts.size());
      // std::cout << fit_id << std::endl;
      int id = 0;
      for (auto & gen : genotype_lists) {
        fitnesses[fit_id][id] = fit_funs[fit_id](world.GetOrg(gen[0]));
        id++;
      }
    }

    // std::cout << to_string(fitnesses) << std::endl;
  // std::cout << "fitness calculated" << std::endl;
    // Go through a new ordering of fitness functions for each selections.
  // std::cout << "randdomized" << std::endl;
    for (size_t repro = 0; repro < repro_count; ++repro) {
  // std::cout << "repro: " << repro << std::endl;
      // Determine the current ordering of the functions.
      emp::vector<size_t> order;

      if (max_funs == fit_funs.size()) {
        order = GetPermutation(world.GetRandom(), fit_funs.size());
      } else {
        order.resize(max_funs); // We want to limit the total numebr of tests done.
        for (auto & x : order) x = world.GetRandom().GetUInt(fit_funs.size());
      }
      // @CAO: We could have selected the order more efficiently!
  // std::cout << "reoreder" << std::endl;
      // Step through the functions in the proper order.
      cur_gens = all_gens;  // Start with all of the organisms.
      int depth = -1;
      for (size_t fit_id : order) {
        // std::cout << "fit_id: " << fit_id << std::endl;
        depth++;

        // std::cout << "about to index" << std::endl;
        // std::cout << to_string(fitnesses[fit_id]) << std::endl;
        // std::cout << cur_orgs[0] << std::endl;
        double max_fit = fitnesses[fit_id][cur_gens[0]];
        next_gens.push_back(cur_gens[0]);
        // std::cout << "Starting max: " << max_fit << to_string(cur_gens) << std::endl;
        for (size_t gen_id : cur_gens) {
              
          const double cur_fit = fitnesses[fit_id][gen_id];
          // std::cout << "gen_id: " << gen_id << "Fit: " << cur_fit << std::endl;
          if (cur_fit > max_fit) {
            max_fit = cur_fit;             // This is a the NEW maximum fitness for this function
            next_gens.resize(0);           // Clear out orgs with former maximum fitness
            next_gens.push_back(gen_id);   // Add this org as only one with new max fitness
            // std::cout << "New max: " << max_fit << " " << cur_gens.size() << std::endl;
          }
          else if (cur_fit == max_fit) {
            next_gens.push_back(gen_id);   // Same as cur max fitness -- save this org too.
            // std::cout << "Adding: " << gen_id << std::endl;
          }
        }
        // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
        std::swap(cur_gens, next_gens);
        next_gens.resize(0);
        
        if (cur_gens.size() == 1) break;  // Stop if we're down to just one organism.
      }

      // Place a random survivor (all equal) into the next generation!
      emp_assert(cur_gens.size() > 0, cur_gens.size(), fit_funs.size(), all_gens.size());
      size_t options = 0;
      for (size_t gen : cur_gens) {
        options += genotype_lists[gen].size();
      }
      size_t winner = world.GetRandom().GetUInt(options);
      int repro_id = -1;

      for (size_t gen : cur_gens) {
        if (winner < genotype_lists[gen].size()) {
          repro_id = genotype_lists[gen][winner];
          break;
        }
        winner -= genotype_lists[gen].size();
      }
      emp_assert(repro_id != -1, repro_id, winner, options);

      // std::cout << depth << "abotu to calc used" <<std::endl;
      emp::vector<size_t> used = Slice(order, 0, depth+1);
      // If the world has a OnLexicaseSelect method, call it
      // std::cout << depth << " " << to_string(used) << std::endl;
      TriggerOnLexicaseSelect(world, used, repro_id);
      world.DoBirth( world.GetGenomeAt(repro_id), repro_id );
    }
    // std::cout << "Done with lex" << std::endl;
  }

    // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
    // functions.  The best individuals on each supplemental function divide up a resource pool.
    // NOTE: You must turn off the FitnessCache for this function to work properly.
    template<typename ORG>
    void EcoSelect(World<ORG> & world, const emp::vector<std::function<double(const ORG &)> > & extra_funs,
                   const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp_assert(world.GetFitFun(), "Must define a base fitness function");
      emp_assert(world.GetSize() > 0);
      emp_assert(t_size > 0 && t_size <= world.GetSize(), t_size, world.GetSize());
      // emp_assert(world.IsCacheOn() == false, "Ecologies mean constantly changing fitness!");

      if (world.IsCacheOn()) {
          world.ClearCache();
      }

      // Setup info to track fitnesses.
      emp::vector<double> base_fitness(world.GetSize());
      emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
      emp::vector<double> max_extra_fit(extra_funs.size(), 0.0);
      emp::vector<size_t> max_count(extra_funs.size(), 0);
      for (size_t i=0; i < extra_funs.size(); i++) {
        extra_fitnesses[i].resize(world.GetSize());
      }

      // Collect all fitness info.
      for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
        base_fitness[org_id] = world.CalcFitnessID(org_id);
        for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
          double cur_fit = extra_funs[ex_id](world[org_id]);
          extra_fitnesses[ex_id][org_id] = cur_fit;
          if (cur_fit > max_extra_fit[ex_id]) {
            max_extra_fit[ex_id] = cur_fit;
            max_count[ex_id] = 1;
          }
          else if (cur_fit == max_extra_fit[ex_id]) {
            max_count[ex_id]++;
          }
        }
      }

      // Readjust base fitness to reflect extra resources.
      for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
        if (max_count[ex_id] == 0) continue;  // No one gets this reward...

        // The current bonus is divided up among the organisms that earned it...
        const double cur_bonus = pool_sizes[ex_id] / max_count[ex_id];
        // std::cout << "Bonus " << ex_id << " = " << cur_bonus
        //           << "   max_extra_fit = " << max_extra_fit[ex_id]
        //           << "   max_count = " << max_count[ex_id]
        //           << std::endl;

        for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
          // If this organism is the best at the current resource, git it the bonus!
          if (extra_fitnesses[ex_id][org_id] == max_extra_fit[ex_id]) {
            base_fitness[org_id] += cur_bonus;
          }
        }
      }


      emp::vector<size_t> entries;
      for (size_t T = 0; T < tourny_count; T++) {
        entries.resize(0);
        for (size_t i=0; i<t_size; i++) entries.push_back( world.GetRandomOrgID() ); // Allows replacement!

        double best_fit = base_fitness[entries[0]];
        size_t best_id = entries[0];

        // Search for a higher fit org in the tournament.
        for (size_t i = 1; i < t_size; i++) {
          const double cur_fit = base_fitness[entries[i]];
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = entries[i];
          }
        }

        // Place the highest fitness into the next generation!
        world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
      }
    }

    /// EcoSelect can be provided a single value if all pool sizes are identical.
    template<typename ORG>
    void EcoSelect(World<ORG> & world, const emp::vector<typename World<ORG>::fun_calc_fitness_t > & extra_funs,
                   double pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp::vector<double> pools(extra_funs.size(), pool_sizes);
      EcoSelect(world, extra_funs, pools, t_size, tourny_count);
    }

  template<typename ORG>
  struct MapElitesPhenotype {
    using pheno_fun_t = std::function<size_t(const ORG &)>;

    pheno_fun_t pheno_fun;   ///< Function to categorize org into phenotype id.
    size_t id_count;         ///< Numbe of phenotype categories.

    MapElitesPhenotype() : pheno_fun_t(), id_count(0) { ; }
    MapElitesPhenotype(const pheno_fun_t & _f, size_t _ids) : pheno_fun_t(_f), id_count(_ids) { ; }

    bool OK() const { return pheno_fun && id_count; }

    size_t GetID(const ORG & org) const {
      size_t id = pheno_fun(org);
      emp_assert(id < id_count);
      return id;
    }
  };

  template<typename ORG>
  struct MapElitesConfig {
    emp::vector< MapElitesPhenotype<ORG> > phenotypes; ///< Funs to categorizes orgs into phenotypes.

    bool OK() const { for (auto & p : phenotypes) if (!p.OK()) return false; return true; }

    size_t GetID(const ORG & org) const {
      size_t id = 0, scale = 1;
      for (const auto & p : phenotypes) {
        const size_t pid = p.GetID(org);
        id += pid * scale;
        scale *= p.id_count;
      }
      return id;
    }

    size_t GetIDCount() const {
      size_t id_count = 1;
      for (const auto & p : phenotypes) id_count *= p.id_count;
      return id_count;
    }
  };

  /// ==MAP-ELITES== Add a new organism to MapElites.  Selection looks at multiple phenotypic
  /// traits and keeps only the highest fitness with each combination of traits.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param config Information about the pheonotypes that Map Elites needs to use.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  template<typename ORG>
  void MapElitesSeed(World<ORG> & world,
                     const MapElitesConfig<ORG> & config,
                     const ORG & org)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(config.OK());
  }

  /// ==MAP-ELITES== Replicate a random organism in MapElites.  Selection looks at multiple
  /// phenotypic traits and keeps only the highest fitness with each combination of traits.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param config Information about the pheonotypes that Map Elites needs to use.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  template<typename ORG>
  void MapElitesGrow(World<ORG> & world,
                     const MapElitesConfig<ORG> & config,
                     size_t repro_count=1)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(config.OK());
  }

}

#endif
