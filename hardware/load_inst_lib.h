#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>
using namespace std::placeholders;

#include "HardwareCPU.h"
#include "InstLib.h"

namespace emp {

  // void Load4StackDefault(InstLib<HardwareCPU<>, Instruction> & inst_lib) {
  template <int CPU_SCALE=8, int STACK_SIZE=16>
  void Load4StackDefault(InstLib<HardwareCPU<CPU_SCALE, STACK_SIZE>, Instruction> & inst_lib) {
    // Load as many nops as we need.  This we be called Nop-0, Nop-1, Nop-2, etc.
    for (int i = 0; i < CPU_SCALE; i++) {
      std::string inst_name = "Nop-";
      inst_name += std::to_string(i);
      inst_lib.AddInst(inst_name, std::bind(&HardwareCPU<>::Inst_Nop, _1), i, 1);
    }

    // Load in single-argument math operations.
    inst_lib.AddInst("Inc", std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a+1;}));
    inst_lib.AddInst("Dec", std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a-1;}));
    inst_lib.AddInst("Shift-L", std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a<<1;}));
    inst_lib.AddInst("Shift-R", std::bind(&HardwareCPU<>::Inst_1I_Math<1,0>, _1, [](int a){return a>>1;}));

    // Load in double-argument math operations.
    inst_lib.AddInst("Nand", std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                       [](int a, int b) { return ~(a&b); }));
    inst_lib.AddInst("Add",  std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                       [](int a, int b) { return a+b; }));
    inst_lib.AddInst("Sub",  std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                       [](int a, int b) { return a-b; }));
    inst_lib.AddInst("Mult", std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                       [](int a, int b) { return a*b; }));

    // @CAO For the next two, ideally if b==0, we should have the instruction return false...
    inst_lib.AddInst("Div", std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                      [](int a, int b) { return (b==0)?0:a/b; }));
    inst_lib.AddInst("Mod", std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,1>, _1,
                                      [](int a, int b) { return (b==0)?0:a%b; }));

    // Conditionals
    inst_lib.AddInst("Test-Equal",  std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,7>, _1,
                                              [](int a, int b) { return a == b; }));
    inst_lib.AddInst("Test-NEqual", std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,7>, _1,
                                              [](int a, int b) { return a != b; }));
    inst_lib.AddInst("Test-Less",   std::bind(&HardwareCPU<>::Inst_2I_Math<1,1,7>, _1,
                                              [](int a, int b) { return a < b; }));
    inst_lib.AddInst("Test-AtStart", std::mem_fn(&HardwareCPU<>::Inst_TestAtStart));

    // Load in Jump operations  [we neeed to do better...  push and pop heads?]
    inst_lib.AddInst("Jump",       std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<0, 3>));
    inst_lib.AddInst("Jump-If0",   std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,7>, _1,
                                             [](int a) { return a==0; }));
    inst_lib.AddInst("Jump-IfN0",   std::bind(&HardwareCPU<>::Inst_MoveHeadToHeadIf<0,3,7>, _1,
                                              [](int a) { return a!=0; }));
    inst_lib.AddInst("Bookmark",   std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToHead<3, 0>));
    inst_lib.AddInst("Set-Memory", std::mem_fn(&HardwareCPU<>::Inst_MoveHeadToMem<2, 1>));
    // "Find-Label" ********** - Jumps the flow head to a complement label (?...) in its current memory.    

    // Juggle stack contents
    inst_lib.AddInst("Val-Move", std::bind(&HardwareCPU<>::Inst_1I_Math<1,1>, _1, [](int a){return a;}));
    inst_lib.AddInst("Val-Copy", std::bind(&HardwareCPU<>::Inst_1I_Math<1,1,false>, _1,
                                           [](int a){return a;}));
    inst_lib.AddInst("Val-Delete", std::mem_fn(&HardwareCPU<>::Inst_ValDelete));

    // Load in "Biological" instructions
    // "Divide" **********      - Moves memory space 1 (?1) into its own organism.  Needs callback!
    // "Build-Inst" **********  - Works like copy, but pushes instruction to end of memory space.
    // "Get-Input" **********   - Needs callback
    // "Get-Output" **********  - Needs callback
    // "Inject" ?? **********   - Needs callback
  }

};

#endif
