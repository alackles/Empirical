#include <iostream>
#include <vector>
#include <string>

#include "web/init.h"
#include "web/Document.h"
#include "d3_init.h"
#include "scales.h"
#include "utils.h"
#include "../js_utils.h"


namespace UI = emp::web;
UI::Document doc("emp_d3_test");

//***********************************//    
//******** Continuous Scales ********//
//***********************************// 

// scaleLinear
struct TestLinearScale {
  TestLinearScale() {
    std::cout << "------Linear Test Begin------" << std::endl;

    EM_ASM({
      var x = d3.scaleLinear()
        .domain([10, 130])
        .range([0, 960]);

      console.log(x(20)); // 80
      console.log(x(50)); // 320
      console.log(x.invert(80)); // 20
      console.log(x.invert(320)); // 50

      var color = d3.scaleLinear()
        .domain([10, 100])
        .range(["brown", "steelblue"]);

      console.log(color(20)); // "#9a3439" or "rgb(154, 52, 57)"
      console.log(color(50)); // "#7b5167" or "rgb(123, 81, 103)"
      
      //
      //??? THIS DOESN"T WORK ON JS SIDE
      //

      console.log(color.invert("#9a3439")); // 20
      console.log(color.invert("#7b5167")); // 50
    });

    D3::LinearScale testLinearX = D3::LinearScale();
    testLinearX.SetDomain(10, 130);
    testLinearX.SetRange(0, 960);
    int result1 = testLinearX.ApplyScaleInt(20);
    int result2 = testLinearX.ApplyScaleInt(50);
    int result1i = testLinearX.Invert<int>(80);
    int result2i = testLinearX.Invert<int>(320);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 1 invert: " << result1i << std::endl;
    std::cout << "value 2 invert: " << result2i << std::endl;
    
    D3::LinearScale testLinearColor = D3::LinearScale();
    testLinearColor.SetDomain(10, 100);
    emp::array<std::string, 2> colorArray = {"brown", "steelblue"};
    testLinearColor.SetRange(colorArray);
    std::string result3 = testLinearColor.ApplyScaleString(20);
    std::string result4 = testLinearColor.ApplyScaleString(50);
    // double result3i = testLinearColor.Invert<std::string>("rgb(154, 52, 57)");
    // double result4i = testLinearColor.Invert<std::string>("rgb(123, 81, 103)");

    std::cout << "value 3: " << result3 << std::endl;
    std::cout << "value 4: " << result4 << std::endl;
    // std::cout << "value 3 invert: " << result3i << std::endl;
    // std::cout << "value 4 invert: " << result4i << std::endl;

    std::cout << "------Linear Test End------" << std::endl << std::endl;
  }
};

// scalePow
struct TestPowScale {
  TestPowScale() {
    std::cout << "------Pow Test Begin------" << std::endl;

    EM_ASM({
      var population = d3.scalePow()
        .exponent(0.5)
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population(1.386e9)); // 249.73986465920893
      console.log(population(127e6)); // 75.59761901012492
      console.log(population(427e3)); // 4.383491758860737

      var population2 = d3.scalePow()
        .exponent(1.5)
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population2(1.386e9)); // 249.73986465920893
      console.log(population2(127e6)); // 75.59761901012492
      console.log(population2(427e3)); // 4.383491758860737

      var populationColor = d3.scalePow()
        .domain([0, 2e9])
        .range(["yellow", "red"]);

      console.log(populationColor(1.386e9)); // 249.73986465920893
      console.log(populationColor(127e6)); // 75.59761901012492
      console.log(populationColor(427e3)); // 4.383491758860737
    });

    D3::PowScale testPowPop = D3::PowScale();
    testPowPop.SetExponent(0.5);
    testPowPop.SetDomain(0, 2e9);
    testPowPop.SetRange(0, 300);
    double result1 = testPowPop.ApplyScaleDouble(1.386e9);
    double result2 = testPowPop.ApplyScaleDouble(127e6);
    double result3 = testPowPop.ApplyScaleDouble(427e3);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    D3::PowScale testPowPop2 = D3::PowScale();
    testPowPop2.SetExponent(1.5);
    testPowPop2.SetDomain(0, 2e9);
    testPowPop2.SetRange(0, 300);
    double result4 = testPowPop2.ApplyScaleDouble(1.386e9);
    double result5 = testPowPop2.ApplyScaleDouble(127e6);
    double result6 = testPowPop2.ApplyScaleDouble(427e3);

    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;
    std::cout << "value 6: " << result6 << std::endl;

    D3::PowScale testPowPopColor = D3::PowScale();
    emp::array<std::string, 2> colorArray = {"yellow", "red"};
    testPowPopColor.SetDomain(0, 2e9);
    testPowPopColor.SetRange(colorArray);
    std::string result7 = testPowPopColor.ApplyScaleString(1.386e9);
    std::string result8 = testPowPopColor.ApplyScaleString(127e6);
    std::string result9 = testPowPopColor.ApplyScaleString(427e3);

    std::cout << "value 7: " << result7 << std::endl;
    std::cout << "value 8: " << result8 << std::endl;
    std::cout << "value 9: " << result9 << std::endl;
   
    std::cout << "------Pow Test End------" << std::endl << std::endl;
  }
};

// scaleSqrt
struct TestSqrtScale {
  TestSqrtScale() {
    std::cout << "------Sqrt Test Begin------" << std::endl;

    EM_ASM({
      var population = d3.scaleSqrt()
        .domain([0, 2e9])
        .range([0, 300]);

      console.log(population(1.386e9)); // 249.73986465920893
      console.log(population(127e6)); // 75.59761901012492
      console.log(population(427e3)); // 4.383491758860737
    });

    D3::PowScale testSqrtPop = D3::PowScale();
    // testSqrtPop = testSqrtPop.SqrtScale();
    testSqrtPop.SqrtScale();
    testSqrtPop.SetDomain(0, 2e9);
    testSqrtPop.SetRange(0, 300);
    double result1 = testSqrtPop.ApplyScaleDouble(1.386e9);
    double result2 = testSqrtPop.ApplyScaleDouble(127e6);
    double result3 = testSqrtPop.ApplyScaleDouble(427e3);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    std::cout << "------Sqrt Test End------" << std::endl << std::endl;
  }
};

// scaleLog
struct TestLogScale {
  TestLogScale() {
    std::cout << "------Log Test Begin------" << std::endl;

    EM_ASM({
      var logScale = d3.scaleLog()
	      .domain([10, 100000])
	      .range([0, 700]);

      console.log(logScale(1000)); // 349.99999999999994
      console.log(logScale(1234)); // 365.980152947014
      console.log(logScale(100000)); // 700

      var logScale2 = d3.scaleLog()
        .base(2)
	      .domain([16, 1048576])
	      .range([0, 700]);

      console.log(logScale2(64)); // 87.49999999999999
      console.log(logScale2(1234)); // 274.2742922127871
      console.log(logScale2(1048576)); // 700
    });

    D3::LogScale testLog = D3::LogScale();
    testLog.SetDomain(10, 100000);
    testLog.SetRange(0, 700);
    double result1 = testLog.ApplyScaleDouble(1000);
    double result2 = testLog.ApplyScaleDouble(1234);
    double result3 = testLog.ApplyScaleDouble(100000);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    D3::LogScale testLog2 = D3::LogScale();
    testLog2.Base(2);
    testLog2.SetDomain(16, 1048576);
    testLog2.SetRange(0, 700);
    double result4 = testLog2.ApplyScaleDouble(64);
    double result5 = testLog2.ApplyScaleDouble(1234);
    double result6 = testLog2.ApplyScaleDouble(1048576);

    std::cout << "value 4: " << result4 << std::endl;
    std::cout << "value 5: " << result5 << std::endl;
    std::cout << "value 6: " << result6 << std::endl;

    std::cout << "------Log Test End------" << std::endl << std::endl;
  }
};

// scaleIdentity
struct TestIdentityScale {
  TestIdentityScale() {
    std::cout << "------Identity Test Begin------" << std::endl;

    EM_ASM({
      var identityScale = d3.scaleIdentity()
        .domain([ 12, 1234 ]);

      console.log(identityScale(12)); // 12
      console.log(identityScale(50.6789)); // 50.6789
      console.log(identityScale(1234)); // 1234
    });

    D3::IdentityScale testId = D3::IdentityScale();
    testId.SetDomain(12, 1234);
    double result1 = testId.ApplyScaleDouble(12);
    double result2 = testId.ApplyScaleDouble(50.6789);
    double result3 = testId.ApplyScaleDouble(1234);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 3: " << result3 << std::endl;

    std::cout << "------Identity Test End------" << std::endl << std::endl;
  }
};

// scaleTime
struct TestTimeScale {
  TestTimeScale() {
    std::cout << "------Time Test Begin------" << std::endl;

    EM_ASM({
      var timeScale = d3.scaleTime()
        .domain([new Date(2000, 0, 1), new Date(2000, 0, 2)])
        .range([0, 960]);

      console.log(timeScale(new Date(2000, 0, 1,  5))); // 200
      console.log(timeScale(new Date(2000, 0, 1, 16))); // 640
      console.log(timeScale.invert(200)); // Sat Jan 01 2000 05:00:00 GMT-0800 (PST)
      console.log(timeScale.invert(640)); // Sat Jan 01 2000 16:00:00 GMT-0800 (PST)
    });

    D3::TimeScale testTime = D3::TimeScale();
    emp::array<std::string, 2> dateArray = {"2000-0-1", "2000-0-2"};
    testTime.SetDomain(dateArray);
    testTime.SetRange(0, 960);
    double result1 = testTime.ApplyScaleDouble("2000-0-1-5");
    double result2 = testTime.ApplyScaleDouble("2000-0-1-16");
    double result1i = testTime.Invert<int>(200);
    double result2i = testTime.Invert<int>(640);

    std::cout << "value 1: " << result1 << std::endl;
    std::cout << "value 2: " << result2 << std::endl;
    std::cout << "value 1 invert: " << result1i << std::endl;
    std::cout << "value 2 invert: " << result2i << std::endl;

    std::cout << "------Time Test End------" << std::endl << std::endl;
  }
};


//***********************************//    
//******** __________ Scales ********//
//***********************************// 

// struct TestQuantizeScale {
//   TestQuantizeScale() {
//     EM_ASM({
//       var color = d3.scaleQuantize()
//         .domain([0, 1])
//         .range(["brown", "steelblue"]);

//       console.log(color(0.49)); // "brown"
//       console.log(color(0.51)); // "steelblue"
//     )}; 

//     D3::QuantizeScale testQuantize = D3::QuantizeScale();
//     emp::array<std::string, 2> colors = {"brown", "steelblue"};
//     testQuantize.SetDomain(0, 1);
//     // quantScale.SetRange({"brown", "steelblue"});
//     testQuantize.SetRange(colors);

//     std::string value1 = testQuantize.ApplyScaleString(0.49);
//     std::string value2 = testQuantize.ApplyScaleString(0.51);

//     std::cout << "value 1: " << value1 << std::endl;
//     std::cout << "value 2: " << value2 << std::endl;
//   }
// };


//***********************************//    
//********     Run tests     ********//
//***********************************// 
int main() {

  D3::internal::get_emp_d3();

  // ContinuousScale tests
  TestLinearScale test1{};
  TestPowScale test2{};
  TestSqrtScale test3{};
  TestLogScale test4{};
  TestIdentityScale test5{};
  TestTimeScale test6{};
  // TestRadialScale test7{};

  // TestQuantizeScale test6{};
}



// scaleRadial
// struct TestRadialScale {
//   TestRadialScale() {
//     std::cout << "------Radial Test Begin------" << std::endl;

//     EM_ASM({
//       var radialScale = d3.scaleRadial();

//       console.log(radialScale(0.10)); // 12
//       console.log(radialScale(0.777)); // 50.6789
//       console.log(radialScale(1.000)); // 1234
//     });

//     // D3::RadialScale testRad = D3::RadialScale();
//     // // testRad.SetDomain(10, 1000);
//     // testRad.SetRange(30, 300);
//     // double result1 = testRad.ApplyScaleDouble(10);
//     // double result2 = testRad.ApplyScaleDouble(777);
//     // double result3 = testRad.ApplyScaleDouble(1000);

//     // std::cout << "value 1: " << result1 << std::endl;
//     // std::cout << "value 2: " << result2 << std::endl;
//     // std::cout << "value 3: " << result3 << std::endl;

//     std::cout << "------Radial Test End------" << std::endl << std::endl;
//   }
// };

