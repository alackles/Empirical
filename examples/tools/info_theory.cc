//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using info theory tools.

#include <iostream>

#include "../../tools/info_theory.h"

int main()
{
  std::cout << "Testing!" << std::endl;

  emp::vector<double> vals = {0.25, 0.25, 0.5};
  std::cout << emp::Entropy(vals) << std::endl;
}

