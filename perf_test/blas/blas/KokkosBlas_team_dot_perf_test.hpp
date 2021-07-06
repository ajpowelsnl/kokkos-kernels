

// Created by David Poliakoff and Amy Powell on 6/15/2021


//#ifndef KOKKOSKERNELS_KOKKOSBLAS_DOT_TEST_RPS_HPP
//#define KOKKOSKERNELS_KOKKOSBLAS_DOT_TEST_RPS_HPP
////////////////////////////////////////////////////
#ifndef KOKKOSKERNELS_KOKKOSBLAS_TEAM_DOT_TEST_RPS_HPP
#define KOKKOSKERNELS_KOKKOSBLAS_TEAM_DOT_TEST_RPS_HPP

#include <Kokkos_Core.hpp>
#include "blas/KokkosBlas1_dot.hpp"
#include <Kokkos_Random.hpp>

// These headers are required for RPS perf test implementation
#include <common/KernelBase.hpp>
#include <common/RunParams.hpp>
#include <common/QuickKernelBase.hpp>
#include <PerfTestUtilities.hpp>

// Create a temlated struct 
template <class ExecSpace, class Layout>
struct testData {
  // type aliases declared in the run method of the test
  // implemented in Kokkos Kernels
  using Scalar   = double;
  using MemSpace = typename ExecSpace::memory_space;
  using Device   = Kokkos::Device<ExecSpace, MemSpace>;
  // For the TeamPolicy (using threads) implementation of dot
  using policy = Kokkos::TeamPolicy<ExecSpace>;
  // This type will wil used to create an instance called team
  using member_type = typename policy::member_type;

  // m is vector length                            
  int m           = 100000;         
  int repeat      = 1;              
  bool layoutLeft = true;

  // DECLARATION 
  // Test Matrices x and y, View declaration
  // Create 1D view w/ Device as the ExecSpace; this is an input vector
  Kokkos::View<Scalar*, MemSpace> x("X", m);
  // Create 1D view w/ Device as the ExecSpace; this is the output vector
  Kokkos::View<Scalar*, MemSpace> y("Y", m);

  // A function with no return type whose name is the name of the class is a
  // constructor or a destructor;
  // Constructor -- create function:
  testData(int m) {
          Kokkos::deep_copy(x, 3.0);
          Kokkos::deep_copy(y, 2.0);
  }
};
/////////////////////////////////////////////////////////////////////////////////
// END STRUCT
/////////////////////////////////////////////////////////////////////////////////

/* Taking in by reference avoids making a copy of the data in memory, whereas
 * taking in by value would make a copy in memory.  Copying operations do not
 * enhance performance.
 *  
 *  A function takes data as a pointer when you're dealing with a collection of
 *  things, such as 8 test datasets
 *
 */
// Creating the machinery needed to run as an RPS test

test_list make_team_dot_kernel_base(const rajaperf::RunParams& params);

// Templated function 
template<typename ExecSpace, typename Layout>
testData<ExecSpace, Layout> setup_test(int m,
                                       int repeat,
                                       bool layoutLeft
                                       );

test_list construct_team_dot_kernel_base(const rajaperf::RunParams& run_params);
                                

// Must have the full function body, as templated functions are recipes for
// functions
//
template <class ExecSpace, class Layout>
void run(int m, int repeat) {


  std::cout << "Running BLAS Level 1 TEAMPOLICY-BASED DOT performance experiment ("
            << ExecSpace::name() << ")\n";

  std::cout << "Each test input vector has a length of " << m << std::endl;

  // Declaring variable pool w/ a seeded random number;
  // a parallel random number generator, so you
  // won't get the same number with a given seed each time

  // We're constructing an instance of testData called testMatrices, which takes m as a param
  testData<ExecSpace,Layout> testMatrices(m);

  // team refers to a handle you get from a parallel_for/parallel_reduce;

  // do a warm up run of dot:
  //KokkosBlas::dot(testMatrices.x, testMatrices.y);
  //
  //https://github.com/kokkos/kokkos-kernels/wiki/BLAS-1%3A%3Ateam-dot
  // Multiplies each value of x(i) with y(i) and computes the total within a parallel kernel 
  // using a TeamPolicy execution policy
  
  Kokkos::parallel_for("TeamDotDemoUsage -- WARM UP",
                        // First param = number of teams, aka league size;
                        // The second param is the team size, which Kokkos will
                        // generate for a TeamPolicy and execution space
                        // Kokkos::AUTO = "Kokkos, make your best guess about
                        // how many threads should be in a team
                        policy(1, Kokkos::AUTO),
                        KOKKOS_LAMBDA(const member_type& team) {
                        double team_dot_result = KokkosBlas::Experimental::dot(team, testMatrices.x, testMatrices.y);
                        });






  // The live test of dot:

  Kokkos::fence();
  Kokkos::Timer timer;

    for (int i = 0; i < testMatrices.repeat; i++) {
    double result = KokkosBlas::dot(testMatrices.x, testMatrices.y);
    ExecSpace().fence();
  }

  // Kokkos Timer set up
  double total = timer.seconds();
  double avg   = total / testMatrices.repeat;
  // Flops calculation for a 1D matrix dot product per test run;
  size_t flopsPerRun = (size_t)2 * m;
  printf("Avg DOT time: %f s.\n", avg);
  printf("Avg DOT FLOP/s: %.3e\n", flopsPerRun / avg);
}

#endif //KOKKOSKERNELS_KOKKOSBLAS_DOT_TEST_HPP


