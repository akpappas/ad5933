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
using namespace std::complex_literals;

static cyusb_handle *dev_handle = NULL;
static unsigned int clk=16000000;
//Register Map
const int CTRL_MSB=0x80;
const int CTRL_LSB=0x81;
const int FREQ_23_16=0x82;
const int FREQ_15_8=0x83;
const int FREQ_7_0=0x84;
const int STEP_23_16=0x85;
const int STEP_15_8=0x86;
const int STEP_7_0=0x87;
const int INC_NUM_MSB=0x88;
const int INC_NUM_LSB=0x89;
const int SETTLE_MSB=0x8A;
const int SETTLE_LSB=0x8B;
const int SREG=0x8F;
const int TEMPERATURE_MSB=0x92;
const int TEMPERATURE_LSB=0x93;
const int REAL_MSB=0x94;
const int REAL_LSB=0x95;
const int IMG_MSB=0x96;
const int IMG_LSB=0x97;

/*Control register map*/
/*Bits 15 to 12*/
const int INIT_START_FREQ=0x10;
const int START_FREQ_SWEEP=0x20;
const int INC_FREQ=0x30;
const int REPEAT_FREQ=0x40;
const int MEAS_TEMP=0x90;
const int PD_MODE=0xa0;
const int SB_MODE=0xb0;

/*Output voltage range*/
/*Bits 10 to 9*/
const int OUTPUT_2Vpp=0x00;
const int OUTPUT_200mVpp=0x02;
const int OUTPUT_400mVpp=0x04;
const int OUTPUT_1Vpp=0x06;

/*PGA*/
/*Bit 8*/
const int PGA_GAIN5x=0x00;
const int PGA_GAIN1x=0x01;

/*Bits 7 to 0*/
const int RESET_SET=0x10;
const int CLK_EXT=0x08; /*0: internal; 1: external*/
const int CLK_INT=0x00; /*0: internal; 1: external*/

/*Status register map*/
const int SREG_TEMP_VALID=0x01;
const int SREG_IMPED_VALID=0x02;
const int SREG_SWEEP_VALID=0x04;

/*Settling time map*/
const int SETTLING_MUL_2x = 0x02;
const int SETTLING_MUL_4x = 0x03;
/********************************************************************************/


using std::pair;
using std::make_pair;
using std::vector;
typedef std::complex<long double> complex_t;

void my_error(int err)
{
  fprintf(stderr,"%d: ",err);
  cyusb_error(err);
}

void print_byte(uint8_t b){
  std::bitset<8> p(b);
  std::cout<<p<<"\n";
}

int read_register ( unsigned short reg, unsigned char *buffer, unsigned short len )
{
  auto err = cyusb_control_transfer ( dev_handle,0xc0,0xDE,0x0D,reg,buffer,len,0 );
  if ( err!=0 )
    {
      my_error ( err );
    }
  return err;
}

int write_register ( unsigned char value, unsigned char reg )
{
  auto err = cyusb_control_transfer ( dev_handle,0x40,0xDE,0x0D, value << 8 | reg,NULL,0,0 );
  if ( err!=0 )
    {
      my_error ( err );
    }
  return err;
}

void set_starting_frequency ( uint32_t start )
{
  //Set starting frequency
  uint r2 = ( start & 0xff0000 ) >>16;
  uint r1 = ( start & 0x00ff00 ) >>8;
  uint r0 = ( start & 0x0000ff );
  write_register ( r1, FREQ_23_16 );
  write_register ( r2, FREQ_15_8 );
  write_register ( r0, FREQ_7_0 );
  printf ( "setting starting \n" );
  return;
}

void set_frequency_step ( uint32_t inc )
{
  //Set frequency step
  uint32_t r0,r1,r2;
  r2 = ( inc & 0xff0000 ) >>16;
  r1 = ( inc & 0x00ff00 ) >>8;
  r0 = ( inc & 0x0000ff );
  write_register ( r1, STEP_23_16 );
  write_register ( r2, STEP_15_8 );
  write_register ( r0, STEP_7_0 );
  printf ( "setting step\n" );
}

void set_step_number ( uint32_t number )
{
  //Set number of steps
  uint32_t r0,r1;
  r1 = ( number & 0x00ff00 ) >>8;
  r0 = ( number & 0x0000ff );
  write_register ( r1, INC_NUM_MSB );
  write_register ( r0, INC_NUM_LSB );
  printf ( "setting num\n" );
}

void set_settling_cycles ( uint32_t cycles )
{
  //Set settling time cycles
  uint32_t r0,r1;
  r1 = ( cycles & 0xFF00 ) >>8;
  r0 = ( cycles & 0x00FF );
  write_register ( r1, SETTLE_MSB );
  write_register ( r0, SETTLE_LSB );
  printf ( "Set settling time cycles\n" );
}

void set_voltage_output ( uint8_t& reg, uint8_t command )
{
  reg =reg & 0xF9;
  reg |= command;
  write_register ( reg, CTRL_MSB );
}

void set_PGA ( uint8_t& reg, uint8_t command )
{
  reg &= 0xFE;
  reg |= command;
  write_register ( reg, CTRL_MSB );
}

void set_powermode ( uint8_t& reg, uint8_t command )
{
  reg &= 0x0F;
  reg |= command;
  write_register ( reg, CTRL_MSB );
}

void choose_clock ( uint8_t& reg, uint8_t command )
{
  reg &= 0xF7;
  reg |= command;
  write_register ( reg, CTRL_LSB );
}

std::vector<  std::pair<long double,  complex_t > > sweep_frequency ( uint32_t lower,uint32_t number_of_samples,double step, bool internal )
{
  vector< pair<long double,complex_t >> measurements;
  uint32_t conversion_factor = ( 1<<27 ) / ( clk/4 );
  uint32_t start = lower * conversion_factor;
  uint32_t inc =int ( step * conversion_factor );
  uint16_t settling_cycles = 15;
  set_starting_frequency ( start );
  set_frequency_step ( inc );
  set_step_number ( number_of_samples );
  set_settling_cycles ( settling_cycles );
  //Read Control register
  uint8_t ctrl_reg1=0;
  uint8_t ctrl_reg0=0;
  printf ( "reading ctrl\n" );
  read_register ( CTRL_MSB,&ctrl_reg1,1 );
  read_register ( CTRL_LSB,&ctrl_reg0,1 );
  printf ( "set voltage output\n" );
  set_voltage_output ( ctrl_reg1,OUTPUT_2Vpp );
  printf ( "set pga\n" );
  set_PGA ( ctrl_reg1,PGA_GAIN1x );
  printf ( "choose clock\n" );
  if ( internal )
    {
      choose_clock ( ctrl_reg0, CLK_INT );
    }
  else
    {
      choose_clock ( ctrl_reg0, CLK_EXT );
    }
  printf("set powermode\n");
  set_powermode ( ctrl_reg1,SB_MODE );
  usleep ( 5e5 );
  printf ( "set powermode\n" );
  set_powermode ( ctrl_reg1,INIT_START_FREQ );
  usleep ( 5e5 );
  printf ( "set powermode\n" );
  set_powermode ( ctrl_reg1,START_FREQ_SWEEP );
  usleep( 5e5);
  read_register ( CTRL_MSB,&ctrl_reg1,1 );
  print_byte(ctrl_reg1);
  print_byte(START_FREQ_SWEEP);
  uint8_t sreg=0;
  /*Sweep loop*/
  long double true_freq;
  auto cur_freq = start;
  int i=0;
  printf ( "Measurements (%d)",number_of_samples );
  for ( ;; )
    {
      long double ar= ( cur_freq );
      true_freq = ar/conversion_factor;
      /*Read SREG for valid impedance meausurement*/
      uint8_t impedance_valid=0;
      do
        {
//           printf("checking for valid impedance");
          read_register ( SREG,&sreg,1 );
          usleep((4*16e6)/true_freq);
          std::cout<<(4*16e6)/true_freq<<" ";
//           printf("%d ",sreg);
          impedance_valid = sreg & SREG_IMPED_VALID;
  //        printf("%d\n",impedance_valid);
        }
      while ( !impedance_valid );
      uint8_t re1,re0,im1,im0;
      read_register ( REAL_MSB,&re1,1 );
      read_register ( REAL_LSB,&re0,1 );
      read_register ( IMG_MSB,&im1,1 );
      read_register ( IMG_LSB,&im0,1 );
      int16_t real = re1<<8 | re0;
      int16_t img = im1<<8 | im0;
      long double dreal = real;
      long double dimg = img;
      std::complex<long double> z ( dreal,dimg );
      measurements.push_back ( make_pair ( true_freq,z ) );
      cur_freq+=inc;
      i++;
      read_register ( SREG,&sreg,1 );
      if ( sreg & SREG_SWEEP_VALID ) break;
      set_powermode ( ctrl_reg1, INC_FREQ );
    }
  return measurements;
}

float get_temperature()
{
  cyusb_control_transfer ( dev_handle,0x40,0xDE,0x0D,0x9080,NULL,0,0 );
  float temp=0;
  unsigned char hi,lo,sreg;
  hi=0;
  lo=0;
  sreg=0;
  int temp_valid=0;
  do
    {
      cyusb_control_transfer ( dev_handle,0xc0,0xDE,0x0D,SREG,&sreg,1,0 );
      temp_valid = sreg & SREG_TEMP_VALID;
    }
  while ( !temp_valid );
  cyusb_control_transfer ( dev_handle,0xc0,0xDE,0x0D,TEMPERATURE_MSB,&hi,1,0 );
  cyusb_control_transfer ( dev_handle,0xc0,0xDE,0x0D,TEMPERATURE_LSB,&lo,1,0 );
  if ( hi >> 5 )
    {
      temp = ( ( ( ( hi&0xff ) <<8 ) | ( lo&0xff ) ) - 16384 ) / 32.0;
    }
  else
    {
      temp = ( ( ( hi&0xff ) <<8 ) | ( lo&0xff ) ) /32.0;
    }
  return temp;
}


std::vector<std::pair<long double,long double>> calibrate_gain( std::vector<std::pair<long double,complex_t>> measurements,long double calibration_resistance)
{
  std::vector<std::pair<long double,long double>> gains;
  for (const auto& i: measurements)
  {
    auto temp = std::abs( i.second)*calibration_resistance;
    gains.push_back(std::make_pair(i.first,1/temp));
  }
  return gains;
}


std::vector<std::pair<long double, long double>> calculate_magnitude(std::vector<std::pair<long double,complex_t>> measurements, std::vector<std::pair<long double,long double>> gains)
{
  std::vector<std::pair<long double, long double>> magnitude;
  for (int i=0;i<measurements.size();++i)
  {
    auto temp = std::abs(measurements[i].second)*gains[i].second;
    magnitude.push_back(std::make_pair(measurements[i].first,1/temp));
  }
  return magnitude;
}

int main ( int argc, char **argv )
{
  int r;
  r = cyusb_open ( 0x0456, 0xb203 );
  if ( r < 0 )
    {
      printf ( "Error opening library\n\n" );
      return -1;
    }
  else if ( r == 0 )
    {
      printf ( "No device found\n" );
      return 0;
    }
  if ( r > 1 )
    {
      printf ( "More than 1 devices of interest found. Disconnect unwanted devices\n" );
      return 0;
    }
  dev_handle = cyusb_gethandle ( 0 );
  if ( cyusb_getvendor ( dev_handle ) != 0x0456 )
    {
      printf ( "Cypress chipset not detected\n" );
      cyusb_close();
      return 0;
    }
  if ( cyusb_getproduct ( dev_handle ) != 0xb203 )
    {
      printf ( "Cypress chipset not detected\n" );
      cyusb_close();
      return 0;
    }
  r = cyusb_kernel_driver_active ( dev_handle, 0 );
  if ( r != 0 )
    {
      printf ( "kernel driver active. Exitting\n" );
      cyusb_close();
      return 0;
    }
  r = cyusb_claim_interface ( dev_handle, 0 );
  if ( r != 0 )
    {
      printf ( "Error in claiming interface\n" );
      cyusb_close();
      return 0;
    }
  else printf ( "Successfully claimed interface\n" );
  static unsigned char extension;
  extension = strtoul ( "0xA0", NULL, 16 );
  struct stat statbuf;
  static char filename[] = "./AD5933_34FW.hex";
  r = stat ( filename, &statbuf );
  printf ( "File size = %d\n", ( int ) statbuf.st_size );
  r = cyusb_download_fx2 ( dev_handle, filename, extension );
  if ( r )
    {
      printf ( "Error: %d\n",r );
      my_error ( r );
    }
  auto temp=get_temperature();
  printf ( "temperature=%f\n",temp );
  printf ( "starting\n" );
  auto admitance = sweep_frequency ( 3e5,10,10,false );
  auto gains=calibrate_gain(admitance,1000);
  std::vector<std::pair<long double,long double>> system_phase;
  for (const auto& i: admitance)
  {
   system_phase.push_back(std::make_pair<i.first,std::arg(i.second));
  }
  std::cout<<"Change\n";
  std::string garbage;
  std::cin>>garbage;
  std::cout<<"\n";
  auto newZ = sweep_frequency ( 3e5,10,10,false );

  auto mag = calculate_magnitude(newZ,gains);
  for (int i=0;i<mag.size();++i)
  {
    std::cout<<mag[i].first<<": "<<mag[i].second<<" "<<std::abs(admitance[i].second)/std::abs(newZ[i].second)<<"\n";
  }
  return 0;
}
