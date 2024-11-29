#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <random>
#include <vector>

#include "mpi/laganina_e_readers_writers/include/ops_mpi.hpp"

std::vector<int> laganina_e_readers_writers_mpi::getRandomVector(int sz) {
  std::random_device dev;
  std::mt19937 gen(dev());
  std::vector<int> vec(sz);
  for (int i = 0; i < sz; i++) {
    vec[i] = (gen() % 100) - 49;
  }
  return vec;
}

TEST(laganina_e_readers_writers_mpi, Test_1_second) {
  boost::mpi::communicator world;
  
  int time=1;
  int out=0;
  
  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();

  if (world.rank() == 0) {
    taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(&time));
    taskDataPar->outputs.emplace_back(reinterpret_cast<uint8_t*>(&out));
  }

  laganina_e_readers_writers_mpi::TestMPITaskParallel testMpiTaskParallel(taskDataPar);
 
  ASSERT_EQ(testMpiTaskParallel.validation(), true);
  ASSERT_EQ(testMpiTaskParallel.pre_processing(),true);
  ASSERT_EQ(testMpiTaskParallel.run(),true);
  ASSERT_EQ(testMpiTaskParallel.post_processing(),true);
  if(world.size()!=1){
  
  bool correct=(out >= 1 )&& (out<= 999);
  ASSERT_EQ(correct,true);
  }
  else{
	  ASSERT_EQ(true,true);
  }
}

