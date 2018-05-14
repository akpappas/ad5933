#include <stdio.h>
#include <iostream>
#include <complex>
#include <vector>
#include <utility>
#include <getopt.h>
#include <cmath>
#include "ad5933.hpp"


int main ( int argc, char **argv )
{
  using std::make_pair;
  AD5933 analyzer;
  long double start=30000;
  auto temp = analyzer.measure_temperature();
  printf ( "Temperature= %f C\n",temp );
  printf ( "Starting\n" );
  analyzer.choose_clock(Clk::INT);
  analyzer.set_settling_multiplier(SettlingMultiplier::MUL_1x);
  analyzer.set_settling_cycles(15);
  analyzer.set_voltage_output(Voltage::OUTPUT_2Vpp);
  analyzer.set_PGA(Gain::PGA1x);
  auto admitance = sweep_frequency ( start,10,10, &analyzer );
  auto gains=calibrate_gain(admitance,1000);
  std::vector<std::pair<long double,long double>> system_phase;
  printf("System phase:\n");
  for (const auto& i: admitance)
  {
    auto phi = std::arg(i.second);
    phi = phi * (180.0l/M_PIl);
    printf("%Lf\t%Lf\t%Lf\t%Lf\n",
	   phi,std::atan2(i.second.imag(),i.second.real())*(180.0l/M_PIl),
	   i.second.real(),i.second.imag());
    system_phase.push_back(std::make_pair(i.first,phi));
  }
 
  std::cout<<"Change\n";
  std::string garbage;
  std::cin>>garbage;
  std::cout<<"\n";

  
  auto newZ = sweep_frequency ( start,10,10,&analyzer );
  auto mag = calculate_magnitude(newZ,gains);
  std::vector<std::pair<long double,long double>> new_phase;
  std::vector<long double> new_arg;
  printf("new phase");
  for (size_t i = 0; i < newZ.size(); ++i)
  {
    auto phiZ = std::arg(newZ[i].second);
    phiZ = phiZ * (180.0l/M_PIl);
    new_arg.push_back(phiZ);
    if (newZ[i].first!= system_phase[i].first) printf("ooops!%Lf \n",newZ[i].first-system_phase[i].first);
    auto phi0 = phiZ - system_phase[i].second;
    printf("%Lf\t%Lf\t%Lf\n", phi0,phiZ, system_phase[i].second);
    //    new_arg.push_back(phi0);
    //    std::cout<<phi0<<" ";
    new_phase.push_back( make_pair(newZ[i].first, phi0));
  }
  std::cout<<std::endl;
  
  write_to_file(mag, new_phase, newZ);
  
  auto two_point = gains;
  two_point.clear();
  two_point.push_back(gains.front());
  two_point.push_back(gains.back());

  
  auto two_phi = system_phase;
  two_phi.clear();
  two_phi.push_back(system_phase.front());
  two_phi.push_back(system_phase.back());


  auto new_gains = calc_multigains(newZ,two_point);
  auto new_mag = calculate_magnitude(newZ, new_gains);
  //  for (size_t i=0; i<new
  
  for (size_t i=0;i<mag.size();++i)
  {
    std::cout<<mag[i].first<<"\t"<<mag[i].second<<"\t<"<<new_phase[i].second<<"\t< "
	     <<system_phase[i].second<<"\t<"<<new_arg[i]<<std::endl;
  }
}


