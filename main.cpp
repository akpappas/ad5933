#include <stdio.h>
#include <iostream>
#include <complex>
#include <vector>
#include <utility>
#include <getopt.h>

#include "ad5933.hpp"


int main ( int argc, char **argv )
{
  AD5933 analyzer;
  auto temp = analyzer.measure_temperature();
  printf ( "Temperature= %f C\n",temp );
  printf ( "Starting\n" );
  analyzer.choose_clock(Clk::INT);
  analyzer.set_settling_multiplier(SettlingMultiplier::MUL_1x);
  analyzer.set_settling_cycles(15);
  analyzer.set_voltage_output(Voltage::OUTPUT_2Vpp);
  auto admitance = sweep_frequency ( 3e5,10,10, &analyzer );
  auto gains=calibrate_gain(admitance,1000);
  std::vector<std::pair<long double,long double>> system_phase;
  for (const auto& i: admitance)
  {
    system_phase.push_back(std::make_pair(i.first,std::arg(i.second)));
  }
  std::cout<<"Change\n";
  std::string garbage;
  std::cin>>garbage;
  std::cout<<"\n";
  auto newZ = sweep_frequency ( 3e5,10,10,&analyzer );
  auto mag = calculate_magnitude(newZ,gains);
  auto two_point = gains;
  two_point.clear();
  two_point.push_back(gains.front());
  two_point.push_back(gains.back());
  auto new_g = calc_multigains(newZ,two_point);
  auto new_mag = calculate_magnitude(newZ, new_g);
  for (int i=0;i<gains.size();++i)
  {
    std::cout<<gains[i].first<<": "<<gains[i].second<<" "<<new_g[i].second<<std::endl;
  }
  for (int i=0;i<mag.size();++i)
  {
    std::cout<<mag[i].first<<": "<<mag[i].second<<" "<<new_mag[i].second<<std::endl;
  }
}
