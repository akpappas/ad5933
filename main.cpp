#include <stdio.h>
#include <iostream>
#include <complex>
#include <vector>
#include <utility>
#include <getopt.h>
#include <cmath>
#include "ad5933.hpp"



void user_interaction(AD5933 &h)
{
  long double starting_frequency,ending_frequency;
  std::cout<<std::endl;
  char clk;
  do 
    {
      std::cout<<"Internal clk? (y/n)";
      std::cin>>clk;
      if (clk=='y')
	{
	  h.clk = h.int_clk;
	  printf("Chose internal clock (%Lf)\n",h.clk);
	}
      else if (clk=='n')
	{
	  std::cout<<"External clk frequency:";
	  std::cin>>h.ext_clk;
	  h.clk = h.ext_clk;
	  printf("Chose internal clock (%Lf)\n",h.clk);
	}
    } while (clk!='y' &&  clk!='n');
  std::cout<<"Starting Frequency: ";
  std::cin>>starting_frequency;
  std::cout<<"Ending Frequency (0 for steps+interval): ";
  std::cin>>ending_frequency;
  long double steps,interval;
  if (ending_frequency > 0.5)
    {
      while (ending_frequency<starting_frequency)
	{
	  std::cout<<"Invalid frequency! Pick a new ending frequency: ";
	  std::cin>>ending_frequency;
	}
      if (ending_frequency - starting_frequency < 512)
	{
	  steps = ending_frequency - starting_frequency;
	  interval = 1;
	}
      else
	{
	  interval = (ending_frequency - starting_frequency)/512;
	  steps = 511;
	}
    }
  else
    {
      std::cout<<"Sweep interval: ";
      std::cin>>interval;
      std::cout<<"Steps: ";
      std::cin>>steps;
    }
  h.set_starting_frequency(starting_frequency);
  h.set_frequency_step(interval);
  h.set_step_number(steps);
  int choice;
  std::cout<<"Pick excitation voltage range:\n1. 2 Vp-p\n2. 200 mVp-p\n3. 400 mVp-p\n4. 1 Vp-p\n";
  std::cin>>choice;
  std::cout<<"Picked "<<choice<<std::endl;
  switch (choice)
    {
    case 1:
      h.set_voltage_output(Voltage::OUTPUT_2Vpp);
      break;
    case 2:
      h.set_voltage_output(Voltage::OUTPUT_200mVpp);
      break;
    case 3:
      h.set_voltage_output(Voltage::OUTPUT_400mVpp);
      break;
    case 4:
      h.set_voltage_output(Voltage::OUTPUT_1Vpp);
      break;
    default:
      std::cout<<"Fell through to default\n";
      h.set_voltage_output(Voltage::OUTPUT_2Vpp);
    }
  std::cout<<"Value in register:"<<h.show_voltage()<<std::endl;
  int pga;
  do
    {
      std::cout<<"PGA (1/5)x:";
      std::cin>>pga;
    }
  while (pga!=1 && pga!=5);
  if (pga==5)
    {
      h.set_PGA(Gain::PGA5x);
    }
  else
    {
      h.set_PGA(Gain::PGA1x);
    }
  int settling_cycles;
  do
    {
      std::cout<<"Settling cycles: ";
      std::cin>>settling_cycles;
    }while (settling_cycles<0 && settling_cycles>511);
  int multiplier;
  do
    {
      std::cout<<"Settling cycle multiplier: ";
      std::cin>>multiplier;
    }while (multiplier!=1 && multiplier!=2 && multiplier!=4);
  switch (multiplier)
    {
    case 1:
      h.set_settling_multiplier(SettlingMultiplier::MUL_1x);
      break;
    case 2:
      h.set_settling_multiplier(SettlingMultiplier::MUL_2x);
      break;
    case 4:
      h.set_settling_multiplier(SettlingMultiplier::MUL_4x);
      break;
    }
  h.print_command_registers();
  printf("Initial Calibration:\nCalibration Resistor Value: ");
  int rcal;
  std::cin>>rcal;
  auto adm = sweep_frequency(starting_frequency, steps, interval, &h);
  printf("Full point calculation");
  auto gains = calibrate_gain(adm, rcal);
  std::vector<std::pair<long double,long double>> system_phase;   
  for (const auto& i: adm)
    {
      long double phi = std::arg(i.second);
      phi = phi * (180.0l/M_PIl);
      system_phase.push_back(std::make_pair(i.first,phi));
    }
  std::cout<<"Please insert unknown impedance"<<std::endl;
  int nouse;
  std::cin>>nouse;
  auto newZ = sweep_frequency(starting_frequency, steps, interval, &h);
  auto mag = calculate_magnitude(newZ, gains);
  std::vector<std::pair<long double,long double>> new_phase;
  std::vector<long double> new_arg;
  for (size_t i = 0; i < newZ.size(); ++i)
  {
    auto phiZ = std::arg(newZ[i].second);
    phiZ = phiZ * (180.0l/M_PIl);
    auto phi0 = phiZ - system_phase[i].second;
    new_phase.push_back( make_pair(newZ[i].first, phi0));
  }
  write_to_file(mag, new_phase, newZ);
}

int main ( int argc, char **argv )
{
  using std::make_pair;
  AD5933 analyzer;
  auto temp = analyzer.measure_temperature();
  printf ( "Temperature= %f C\n",temp );
  user_interaction(analyzer);
  long double start=30000;
  printf ( "Starting\n" );
  /*
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
  */
}


