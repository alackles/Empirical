//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h


#include "../../web/web_init.h"
#ifdef EMSCRIPTEN
#include "../../emtools/JSWrap.h"
#include "../../../d3-emscripten/selection.h"
#include "../../../d3-emscripten/scales.h"
#include "../../../d3-emscripten/axis.h"
#include "../../web/Animate.h"
#endif

#include <iostream>
#include <functional>

#include "../../evo/NK-const.h"
#include "../../evo/OEE.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/StatsManager.h"


constexpr int K = 3;
constexpr int N = 20;

constexpr int POP_SIZE = 10;
constexpr int UD_COUNT = 1000;
constexpr int TOURNAMENT_SIZE = 5;

using BitOrg = emp::BitSet<N>;


/*double return_d(double d, int i=0, int k=0){
  return d;
}*/

int return_i(double d, int i, int k=0){
  return i;
}

#ifdef EMSCRIPTEN
template <typename WORLD>
void Evolve(WORLD * pop, D3::Selection * circles, std::function<double(BitOrg*)> fit_fun) {
    for (auto org : pop->popM){
        std::cout << *org << std::endl;
    }
    std::cout << "in evolve " << pop->popM.size() << std::endl;
    pop->TournamentSelect(fit_fun, TOURNAMENT_SIZE, POP_SIZE);
    pop->Update();
    pop->MutatePop();
    emp::vector<double> fitnesses = RunFunctionOnContainer(fit_fun, pop->popM);
    circles = new D3::Selection(circles->Data(fitnesses));
    //circles.EnterAppend("circle");
    //circles.SetAttr("cx", "scaled_i");
    circles->Transition().SetAttr("cy", "scaled_d");
}
#endif

emp::Random * r;
emp::evo::NKLandscapeConst<N,K> * landscape;
emp::evo::World<BitOrg> * pop;
std::function<double(BitOrg* org)> fit_fun = [](BitOrg * org){ return landscape->GetFitness(*org);};
emp::web::Animate anim;
D3::LinearScale * x_scale;
D3::LinearScale * fitness_scale;

std::function<double(double, int, int)> scaled_d = [](double d, int i, int k){
    return fitness_scale->ApplyScale(d);
};

std::function<double(double, int, int)> scaled_i = [](double d, int i, int k){
    return x_scale->ApplyScale(i);
};

/*std::function<void(double, int, int)> mousover = [&](double d, int i, int k){
    svg.Append("text").SetText(pop[i]);
};*/

int d_id = emp::JSWrap(scaled_d, "scaled_d");
int i_id = emp::JSWrap(scaled_i, "scaled_i");


#ifdef EMSCRIPTEN
D3::Selection * circles;
std::function<void()> animate_function = [](){Evolve(pop, circles, fit_fun);};
#endif

extern "C" int main()
{

  std::cout << "N: " << N << ", K: " << K << ", POP_SIZE: " << POP_SIZE << ", Selection: " << "Standard_tournament" << ", TournamentSize: " << TOURNAMENT_SIZE << std::endl;
  r = new emp::Random();
  landscape = new emp::evo::NKLandscapeConst<N,K>(*r);
  pop = new emp::evo::World<BitOrg>(*r);

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (int j = 0; j < N; j++) next_org[j] = r->P(0.5);
    pop->Insert(next_org);
  }

  pop->SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );

  pop->SetDefaultFitnessFun(fit_fun);

  emp::vector<double> fitnesses = RunFunctionOnContainer(fit_fun, pop->popM);

  #ifdef EMSCRIPTEN
  emp::web::Initialize();
  std::cout << n_objects() << std::endl;
  D3::Selection svg = D3::Selection("body").Append("svg");
  svg.SetAttr("height", 500);
  svg.SetAttr("width", 500);
  circles = new D3::Selection(svg.SelectAll("circle").Data(fitnesses));
  circles->EnterAppend("circle");
  std::cout << "Circles appended" << std::endl;

  fitness_scale = new D3::LinearScale();
  x_scale = new D3::LinearScale();
  fitness_scale->SetDomain(std::array<double, 2>({30, 5}));
  fitness_scale->SetRange(std::array<double, 2>({10, 490}));
  D3::Axis<D3::LinearScale> ax;
  ax.SetScale(*fitness_scale);
  ax.SetOrientation("right");
  ax.Draw(svg);

  x_scale->SetDomain(std::array<double, 2>({0, POP_SIZE-1}));
  x_scale->SetRange(std::array<double, 2>({40, 490}));

  circles->SetAttr("r", 5);
  circles->SetAttr("cx", "scaled_i");
  circles->SetAttr("cy", "scaled_d");
  circles->SetStyle("fill", "green");
  //circles = circles.Transition();
  anim.SetCallback(animate_function);
  anim.Start();
  #endif


/*  // Loop through updates
  for (int ud = 0; ud < UD_COUNT+1; ud++) {
    //// Print current state.
    // for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;
    //std::cout << ud << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
    pop.TournamentSelect(pop.statsM.fit_fun, TOURNAMENT_SIZE, POP_SIZE);
    pop.Update();
    pop.MutatePop();
    // Run a tournament for the rest...
    //std::cout << fitnesses[0] << std::endl;
    #ifdef EMSCRIPTEN
    anim.Step();
    std::cout << ud << std::endl;
    #endif
}*/


  //std::cout << UD_COUNT << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
}
