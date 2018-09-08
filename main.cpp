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
	  h.choose_clock(Clk::INT);
	  printf("Chose internal clock (%Lf)\n",h.clk);
	}
      else if (clk=='n')
	{
	  std::cout<<"External clk frequency:";
	  std::cin>>h.ext_clk;
	  h.clk = h.ext_clk;
	  h.choose_clock(Clk::EXT);
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
  printf("Full point calculation\n");
  auto gains = calibrate_gain(adm, rcal);
  std::vector<std::pair<long double,long double>> system_phase;   
  for (const auto& i: adm)
    {
      long double phi = std::arg(i.second);
      phi = phi * (180.0l/M_PIl);
      system_phase.push_back(std::make_pair(i.first,phi));
    }
  
  for (;;)
    {
      printf("1. Repeat calibration\n2. Measure unknown impendance\n");
      std::cin>>choice;
      if (choice==1)
	{
	  return;
	}
      else if (choice==2)
	{
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
    }
}

int main ( int argc, char **argv )
{
  AD5933 analyzer;
  auto temperature = analyzer.measure_temperature();
  printf ( "Temperature= %f C\n",temperature );
  for (;;)
    {
      user_interaction(analyzer);
    }
}


