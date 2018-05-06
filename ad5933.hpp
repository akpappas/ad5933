#pragma once
#include <cyusb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <complex>
#include <bitset>
#include <vector>
#include <iostream>
#include <libusb-1.0/libusb.h>
//Register Map
const uint8_t CTRL_MSB=0x80;
const uint8_t CTRL_LSB=0x81;
const uint8_t FREQ_23_16=0x82;
const uint8_t FREQ_15_8=0x83;
const uint8_t FREQ_7_0=0x84;
const uint8_t STEP_23_16=0x85;
const uint8_t STEP_15_8=0x86;
const uint8_t STEP_7_0=0x87;
const uint8_t INC_NUM_MSB=0x88;
const uint8_t INC_NUM_LSB=0x89;
const uint8_t SETTLE_MSB=0x8A;
const uint8_t SETTLE_LSB=0x8B;
const uint8_t SREG=0x8F;
const uint8_t TEMPERATURE_MSB=0x92;
const uint8_t TEMPERATURE_LSB=0x93;
const uint8_t REAL_MSB=0x94;
const uint8_t REAL_LSB=0x95;
const uint8_t IMG_MSB=0x96;
const uint8_t IMG_LSB=0x97;

/*Control register map*/
/*Bits 15 to 12*/
enum class Mode
{
  INIT_START_FREQ,
  START_FREQ_SWEEP,
  INC_FREQ,
  REPEAT_FREQ,
  MEAS_TEMP,
  PD_MODE,
  SB_MODE
};
const uint8_t INIT_START_FREQ=0x10;
const uint8_t START_FREQ_SWEEP=0x20;
const uint8_t INC_FREQ=0x30;
const uint8_t REPEAT_FREQ=0x40;
const uint8_t MEAS_TEMP=0x90;
const uint8_t PD_MODE=0xa0;
const uint8_t SB_MODE=0xb0;

/*Output voltage range*/
/*Bits 10 to 9*/
enum class Voltage
{
  OUTPUT_2Vpp,
  OUTPUT_200mVpp,
  OUTPUT_400mVpp,
  OUTPUT_1Vpp
};
const uint8_t OUTPUT_2Vpp=0x00;
const uint8_t OUTPUT_200mVpp=0x02;
const uint8_t OUTPUT_400mVpp=0x04;
const uint8_t OUTPUT_1Vpp=0x06;
/*PGA*/
/*Bit 8*/
enum class Gain{PGA5x,PGA1x};
const uint8_t PGA_GAIN5x=0x00;
const uint8_t PGA_GAIN1x=0x01;

/*Bits 7 to 0*/
const uint8_t RESET_SET=0x10;

enum class Clk {EXT,INT};
const uint8_t CLK_EXT=0x08; /*0: internal; 1: external*/
const uint8_t CLK_INT=0x00; /*0: internal; 1: external*/

/*Status register map*/
const uint8_t SREG_TEMP_VALID=0x01;
const uint8_t SREG_IMPED_VALID=0x02;
const uint8_t SREG_SWEEP_VALID=0x04;

/*Settling time map*/
enum class SettlingMultiplier
{
  MUL_1x,
  MUL_2x,
  MUL_4x
};
const uint8_t SETTLING_MUL_2x = 0x02;
const uint8_t SETTLING_MUL_4x = 0x03;
/********************************************************************************/

using std::pair;
using std::make_pair;
using std::vector;
typedef std::complex<long double> complex_t;

struct AD5933{
  long double ext_clk=4000000l;
  long double int_clk=16776000l;
  long double clk;
  cyusb_handle *h;
  //  std::string filename="./AD5933_34FW.hex";
  uint8_t ctrl_reg1;
  uint8_t ctrl_reg2;
  AD5933();
  int read_register( uint8_t& buffer, uint8_t reg);
  int write_register( uint8_t command,uint8_t reg);
  uint8_t get_status();
  void set_starting_frequency ( uint32_t start );
  void set_frequency_step ( uint32_t inc );
  void set_step_number ( uint32_t number );
  void set_settling_cycles ( uint32_t cycles );
  void set_settling_multiplier (SettlingMultiplier setting);
  void set_voltage_output ( Voltage setting );
  void set_PGA ( Gain setting );
  void set_standby();
  void initilize_frequency();
  void start_sweep();
  void increase_frequency();
  void repeat_frequency();
  double measure_temperature();
  complex_t read_measurement();
  void power_down();
  void choose_clock( Clk setting);
};

void AD5933::set_settling_multiplier(SettlingMultiplier setting)
{
  uint8_t settling_msb=0;
  read_register(settling_msb, SETTLE_MSB);
  settling_msb &= 0xF9; // Clear bit 1:2
  switch (setting)
    {
    case SettlingMultiplier::MUL_2x:
      {
	settling_msb |= SETTLING_MUL_2x;
	break;
      }
    case  SettlingMultiplier::MUL_4x:
      {
	settling_msb |= SETTLING_MUL_4x;
      }
    }
  write_register(settling_msb,SETTLE_MSB);
}

uint8_t AD5933::get_status()
{
  uint8_t buf=0;
  auto err = read_register(buf,SREG);
  if (err<0)
    {
      cyusb_error(err);
      std::abort();
    }
  return buf;
}


void AD5933::choose_clock( Clk setting)
{
  if ( setting == Clk::INT)
  {
    uint8_t mask=0xF7;
    ctrl_reg1 = ctrl_reg1 && mask;
  }
  else
  {
    printf("Chosen external clock: %Lf Hz\n",ext_clk);
    ctrl_reg1 = ctrl_reg1 | CLK_EXT;
  }
  this->write_register(ctrl_reg1, CTRL_LSB);
}

void AD5933::set_starting_frequency(uint32_t start)
{
  uint8_t r2 = ( start & 0xff0000 ) >>16;
  uint8_t r1 = ( start & 0x00ff00 ) >>8;
  uint8_t r0 = ( start & 0x0000ff );
  write_register ( r1, FREQ_23_16 );
  write_register ( r2, FREQ_15_8 );
  write_register ( r0, FREQ_7_0 );
}

void AD5933::set_frequency_step(uint32_t inc)
{
  uint8_t r2 = ( inc & 0xff0000 ) >>16;
  uint8_t r1 = ( inc & 0x00ff00 ) >>8;
  uint8_t r0 = ( inc & 0x0000ff );
  write_register ( r1, STEP_23_16 );
  write_register ( r2, STEP_15_8 );
  write_register ( r0, STEP_7_0 );
}

void AD5933::set_step_number ( uint32_t number )
{
  uint8_t r1 = ( number & 0x00ff00 ) >>8;
  uint8_t r0 = ( number & 0x0000ff );
  write_register ( r1, INC_NUM_MSB );
  write_register ( r0, INC_NUM_LSB );
}

void AD5933::set_settling_cycles ( uint32_t cycles )
{
  uint8_t r1 = ( cycles & 0xFF00 ) >>8;
  uint8_t r0 = ( cycles & 0x00FF );
  write_register ( r1, SETTLE_MSB );
  write_register ( r0, SETTLE_LSB );
}

void AD5933::set_voltage_output ( Voltage setting)
{
  ctrl_reg2 &=0x11111001; // Clear bits 8:9
  if (setting==Voltage::OUTPUT_1Vpp)
      {
	ctrl_reg2 |= OUTPUT_1Vpp;
      }
  else if (setting==Voltage::OUTPUT_200mVpp)
      {
	ctrl_reg2 |= OUTPUT_200mVpp;
      }
  else if (setting==Voltage::OUTPUT_2Vpp)
      {
	ctrl_reg2 |= OUTPUT_2Vpp;
      }
  else if (setting==Voltage::OUTPUT_400mVpp)
      {
	ctrl_reg2 |= OUTPUT_400mVpp;
      }
  this->write_register(ctrl_reg2, CTRL_MSB);
}

void AD5933::set_PGA(Gain setting)
{
  ctrl_reg2 &= 0xFE; // Clear bit 1
  switch (setting)
    {
    case (Gain::PGA1x):
      {
	ctrl_reg2 |= PGA_GAIN1x;
	break;
      }
    case (Gain::PGA5x):
      {
	ctrl_reg2 |= PGA_GAIN5x;
	break;
      }
    }
}

void AD5933::set_standby()
{
  ctrl_reg2 &= 0x0F;
  ctrl_reg2 |= SB_MODE;
  write_register(ctrl_reg2, CTRL_MSB);
}

void AD5933::initilize_frequency()
{
  ctrl_reg2 &= 0x0F;
  ctrl_reg2 |= INIT_START_FREQ;
  write_register(ctrl_reg2, CTRL_MSB);
}

void AD5933::start_sweep()
{
  ctrl_reg2 &= 0x0F;
  ctrl_reg2 |= START_FREQ_SWEEP;
  write_register(ctrl_reg2, CTRL_MSB);
}

void AD5933::increase_frequency()
{
  ctrl_reg2 &= 0x0F;
  ctrl_reg2 |= INC_FREQ;
  write_register(ctrl_reg2, CTRL_MSB);
}

void AD5933::repeat_frequency()
{
  ctrl_reg2 &= 0x0F;
  ctrl_reg2 |= REPEAT_FREQ;
  write_register(ctrl_reg2, CTRL_MSB);
}


int AD5933::write_register ( uint8_t command, uint8_t reg )
{
  auto err = cyusb_control_transfer ( h,0x40,0xDE,0x0D, command << 8 | reg,NULL,0,0 );
  if ( err<0 )
    {
      printf("Error writing 0x%X to register 0x%X",command,reg);
      cyusb_error ( err );
    }
  return err;
}

int AD5933::read_register ( uint8_t &buffer, uint8_t reg )
{
  auto err = cyusb_control_transfer ( h,0xc0,0xDE,0x0D,reg,&buffer,1,0 );
  if ( err<0 )
    {
      printf("Error reading from register 0x%X",reg);
      cyusb_error ( err );
    }
  return err;
}

complex_t AD5933::read_measurement()
{
  uint8_t im1,im0,re1,re0;
  read_register ( re1, REAL_MSB );
  read_register ( re0, REAL_LSB );
  read_register ( im1, IMG_MSB );
  read_register ( im0, IMG_LSB );
  int16_t real = re1<<8 | re0;
  int16_t img = im1<<8 | im0;
  long double dreal = real;
  long double dimg = img;
  complex_t z(dreal,dimg);
  return z;
}


AD5933::AD5933()
{
  auto err = cyusb_open ( 0x0456, 0xb203 );
  if ( err < 0 )
  {
    printf ( "Error opening library\n\n" );
    std::abort();
  }
  else if ( err == 0 )
  {
    printf ( "No device found\n" );
    std::abort();
  }
  if ( err > 1 )
  {
    printf ( "More than 1 devices of interest found. Disconnect unwanted devices\n" );
    std::abort();
  }
  h = cyusb_gethandle(0);
  if ( cyusb_getvendor ( h ) != 0x0456 )
  {
    printf ( "Cypress chipset not detected\n" );
    cyusb_close();
    std::abort();
  }
  if ( cyusb_getproduct ( h ) != 0xb203 )
  {
    printf ( "Cypress chipset not detected\n" );
    cyusb_close();
    std::abort();
  }
  err = cyusb_kernel_driver_active ( h , 0 );
  if ( err != 0 )
  {
    printf ( "kernel driver active. Exitting\n" );
    cyusb_close();
    std::abort();
  }
  err = cyusb_claim_interface ( h, 0 );
  if ( err != 0 )
  {
    printf ( "Error in claiming interface\n" );
    cyusb_close();
    std::abort();
  }
  else printf ( "Successfully claimed interface\n" );
  struct stat statbuf;
  static char filename[] = "./AD5933_34FW.hex";
  err = stat ( filename, &statbuf );
  printf ( "File size = %d\n", ( int ) statbuf.st_size );
  sleep(1);
  auto extension = strtoul ( "0xA0", NULL, 16 );
  err = cyusb_download_fx2 ( h, filename, extension );
  if ( err )
  {
    printf ( "Error downloading firmware: %d\n",err );
  }
  printf("reading ctrl");
  read_register(ctrl_reg1,CTRL_LSB);
  read_register(ctrl_reg2,CTRL_MSB);
  clk=int_clk;
  printf("done constr\n");
}


std::vector<  std::pair<long double,  complex_t > > sweep_frequency ( uint32_t lower,uint32_t number_of_samples,double step, AD5933* h )
{
  vector< pair<long double,complex_t>> measurements;
  uint32_t conversion_factor = ( 1<<27 ) / (h->clk/4 );
  uint32_t start = lower * conversion_factor;
  uint32_t inc =int ( step * conversion_factor );
  h->set_starting_frequency ( start );
  h->set_frequency_step ( inc );
  h->set_step_number ( number_of_samples );

  h->set_standby();
  usleep ( 5e5 );

  h->initilize_frequency();
  usleep ( 5e5 );

  h->start_sweep();
  usleep( 5e5);
  
  /*Sweep loop*/
  long double true_freq;
  auto cur_freq = start;
  int i=0;
  uint8_t sreg;
  for ( ;; )
    {
      long double ar= ( cur_freq );
      true_freq = ar/conversion_factor;
      /*Read SREG for valid impedance meausurement*/
      uint8_t impedance_valid=0;
      int cnt=0;
      do
        {
          usleep(2*(4*16e6)/true_freq);
	  sreg = h->get_status();
	  cnt++;
          impedance_valid = sreg & SREG_IMPED_VALID;
        }
      while ( !impedance_valid );
      std::cout<<"innner loops "<<cnt<<"\n";
      auto z = h->read_measurement();
      measurements.push_back ( make_pair ( true_freq,z ) );
      cur_freq+=inc;
      i++;
      sreg = h->get_status();
      if ( sreg & SREG_SWEEP_VALID ) break;
      h->increase_frequency();
    }
  return measurements;
}

double AD5933::measure_temperature()
{
  uint8_t mask=0x0F;
  ctrl_reg2 &= mask;
  ctrl_reg2 |= MEAS_TEMP;
  write_register(ctrl_reg2, CTRL_MSB);
  uint8_t hi,lo;
  int temp_valid=0;
  double temperature;
  do
    {
      auto sreg =get_status();
      temp_valid = sreg & SREG_TEMP_VALID;
    }
  while ( temp_valid == 0 );
  read_register(hi, TEMPERATURE_MSB);
  read_register(lo, TEMPERATURE_LSB);
  if ( hi >> 5 )
    {
      temperature = ( ( ( ( hi&0xff ) <<8 ) | ( lo&0xff ) ) - 16384 ) / 32.0;
    }
  else
    {
      temperature = ( ( ( hi&0xff ) <<8 ) | ( lo&0xff ) ) /32.0;
    }
  return temperature;
}

std::vector<std::pair<long double,long double>> calibrate_gain(const std::vector<std::pair<long double,complex_t>> &measurements,
							       const long double &calibration_resistance)
{
  std::vector<std::pair<long double,long double>> gains;
  for (const auto& i: measurements)
  {
    auto temp = std::abs( i.second)*calibration_resistance;
    gains.push_back(std::make_pair(i.first,1/temp));
  }
  return gains;
}


std::vector<std::pair<long double, long double>> calculate_magnitude(const std::vector<std::pair<long double,complex_t>> &measurements,
								     const std::vector<std::pair<long double,long double>> &gains)
{
  std::vector<std::pair<long double, long double>> magnitude;
  for (int i=0;i<measurements.size();++i)
  {
    auto temp = std::abs(measurements[i].second)*gains[i].second;
    magnitude.push_back(std::make_pair(measurements[i].first,1/temp));
  }
  return magnitude;
}


long double interpolate(long double f,
			const std::pair<long double, long double> &g0,
			const std::pair<long double, long double>& g1)
{
  long double new_gain = 0;
  if (std::abs(g0.first-f)<0.1)
    {
      new_gain = g0.second;
    }
  else if (std::abs(g1.first-f)<0.1)
    {
      new_gain = g1.second;
    }
  auto frac = (f-g0.first)/(g1.first-g0.first);
  new_gain = (1-frac)*g0.second + frac*g1.second;
  return new_gain;
}


std::vector<std::pair<long double, long double>> calc_multigains(const std::vector<std::pair<long double, complex_t>> &adm,
								 const std::vector<std::pair<long double, long double>> &cal)
{
  std::vector< std::pair<long double, long double>> new_g;
  // Gains were calibrated at higher frequencies, use the gain calibrated at the lowest frequency
  size_t k=0;
  while (k<adm.size() && adm[k].first<cal[0].first)
    {
      new_g.push_back(std::make_pair(adm[k].first,cal[0].second));
      k++;
    }
  // Gains were calibrated at frequencies both higher and lower frequencies. Interpolate between them.
  for (size_t i=0;i<cal.size()-1;i++)
    {
      while (k<adm.size() && adm[k].first<cal[i+1].first)
	{
	  auto temp = interpolate(adm[k].first, cal[i],cal[i+1]);
	  new_g.push_back(std::make_pair(adm[k].first,temp));
	  k++;
	}
    }
  //Gains were calibrated at lower frequencies, use the gain calibrated at the highest frequency.
  while (k<adm.size())
    {
      new_g.push_back(std::make_pair(adm[k].first,cal.back().second));
      k++;
    }
  return new_g;
}
