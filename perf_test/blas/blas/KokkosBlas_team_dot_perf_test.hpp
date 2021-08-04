

#ifndef KOKKOSKERNELS_KOKKOSBLAS_TEAM_DOT_TEST_RPS_HPP
#define KOKKOSKERNELS_KOKKOSBLAS_TEAM_DOT_TEST_RPS_HPP

#include <Kokkos_Core.hpp>
#include "KokkosBlas1_team_dot.hpp"
#include <Kokkos_Random.hpp>

#ifdef KOKKOSKERNELS_ENABLE_TESTS_AND_PERFSUITE
#include <PerfTestUtilities.hpp>
#endif
//  Team Dot documenation
//  https://github.com/kokkos/kokkos-kernels/wiki/BLAS-1%3A%3Ateam-dot 

template <class ExecSpace, class Layout>
struct testData_rps_team_dot {
  
  // type aliases
  using Scalar   = double;
  using MemSpace = typename ExecSpace::memory_space;
  using Device   = Kokkos::Device<ExecSpace, MemSpace>;

  using policy = Kokkos::TeamPolicy<>;
  using member_type = typename policy::member_type;
  int numberOfTeams = 1; // This will be passed to the TeamPolicy
  // Test Matrices x and y, View declaration

    // Declaring 1D view w/ MemSpace as the ExecSpace; this is an input vector
    // DO NOT INITIALIZE
  Kokkos::View<Scalar*, MemSpace> x;

  // Create 1D view w/ Device as the ExecSpace; this is the output vector
  Kokkos::View<Scalar*, MemSpace> y;

  testData_rps_team_dot(int m) {


  x = Kokkos::View<Scalar* , MemSpace>(Kokkos::ViewAllocateWithoutInitializing("x"), m);
  y = Kokkos::View<Scalar* , MemSpace>(Kokkos::ViewAllocateWithoutInitializing("y"), m);

  Kokkos::deep_copy(x, 3.0);
  Kokkos::deep_copy(y, 2.0);

  }
}

/* Taking in by reference avoids making a copy of the data in memory, whereas
 * taking in by value would make a copy in memory.  Copying operations do not
 * enhance performance.
 *  
 *  A function takes data as a pointer when you're dealing with a collection of
 *  things, such as 8 test datasets
 *
 */
// Declaring the machinery needed to run as an RPS test

test_list construct_team_dot_kernel_base(const rajaperf::RunParams& params);

// Templated function 
template<typename ExecSpace, typename Layout>
testData_rps_team_dot<ExecSpace, Layout> setup_test(int m,
                                       int repeat,
                                       bool layoutLeft,
                                       const int numberOfTeams
                                       );

                                

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

  // Constructing an instance of testData, which takes m as a param
  testData_rps_team_dot<ExecSpace,Layout> testMatrices(m);



  // do a warm up run of dot:
  Kokkos::parallel_for("TeamDotUsage_RPS",
                       policy(testMatrices.numberOfTeams, Kokkos::AUTO),
                       KOKKOS_LAMBDA(const typename testData_rps_team_dot<ExecSpace,Layout>::member_type& team) {
                       });


  // The livepo/git_submodules_rps_PR
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

#endif //KOKKOSKERNELS_KOKKOSBLAS_TEAM_DOT_TEST_RPS_HPP

