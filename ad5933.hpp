#pragma once
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#include <cmath>
#include <complex>
#include <bitset>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <sstream>
const uint16_t VID = 0x0456;
const uint16_t PID = 0xb203;

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


const uint8_t MODE_MASK = 0b11110000;
const uint8_t INIT_START_FREQ=0x10;
const uint8_t START_FREQ_SWEEP=0x20;
const uint8_t INC_FREQ=0x30;
const uint8_t REPEAT_FREQ=0x40;
const uint8_t MEAS_TEMP=0x90;
const uint8_t PD_MODE=0xa0;
const uint8_t SB_MODE=0xb0;

std::unordered_map<uint8_t,Mode> mode_map =
  {
    {INIT_START_FREQ,Mode::INIT_START_FREQ},
    {START_FREQ_SWEEP,Mode::START_FREQ_SWEEP},
    {INC_FREQ,Mode::INC_FREQ},
    {REPEAT_FREQ,Mode::REPEAT_FREQ},
    {MEAS_TEMP,Mode::MEAS_TEMP},
    {PD_MODE,Mode::PD_MODE},
    {SB_MODE,Mode::SB_MODE}
  };


/*Output voltage range*/
/*Bits 10 to 9*/
const uint8_t VOLTAGE_MASK = 0b00000110;
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

std::unordered_map<uint8_t,Voltage> voltage_map=
  {
    {OUTPUT_2Vpp,Voltage::OUTPUT_2Vpp},
    {OUTPUT_200mVpp,Voltage::OUTPUT_200mVpp},
    {OUTPUT_400mVpp,Voltage::OUTPUT_400mVpp},
    {OUTPUT_1Vpp,Voltage::OUTPUT_1Vpp}
  };



/*PGA*/
/*Bit 8*/
const uint8_t PGA_MASK = 0b00000001;
enum class Gain{PGA5x,PGA1x};

const uint8_t PGA_GAIN5x=0x00;
const uint8_t PGA_GAIN1x=0x01;

std::unordered_map<uint8_t,Gain> gain_map =
  {
    {PGA_GAIN5x,Gain::PGA5x},
    {PGA_GAIN1x,Gain::PGA1x}
  };


/*Bits 7 to 0*/
const uint8_t RESET_SET=0x10;

const uint8_t CLK_MASK = 0b00000100;
enum class Clk {EXT,INT};
const uint8_t CLK_EXT=0x08; /*0: internal; 1: external*/
const uint8_t CLK_INT=0x00; /*0: internal; 1: external*/

/*Status register map*/
const uint8_t SREG_TEMP_VALID=0x01;
const uint8_t SREG_IMPED_VALID=0x02;
const uint8_t SREG_SWEEP_VALID=0x04;


std::string show_status(uint8_t reg)
{
  if (reg == SREG_IMPED_VALID)
    {
      return "Valid impedance";
    }
  else if (reg == SREG_SWEEP_VALID)
    {
      return "Valid sweep";
    }
  else if (reg == SREG_TEMP_VALID)
    {
      return "Valid temperature";
    }
  else
    {
      return "Invalid status";
    }
}

/*Settling time map*/
const uint8_t MUL_MASK = 0b00000110;
enum class SettlingMultiplier
{
  MUL_1x,
  MUL_2x,
  MUL_4x
};

const uint8_t SETTLING_MUL_2x = 0x02;
const uint8_t SETTLING_MUL_4x = 0x03;

std::string show_multiplier(uint8_t reg)
{
  reg &= MUL_MASK;
  if (reg==SETTLING_MUL_2x)
    {
      return "Mul 2x";
    }
  else if (reg == SETTLING_MUL_4x)
    {
      return "Mul 4x";
    }
  else if (reg == 0)
    {
      return "Mul 1x";
    }
  else
    {
      return "Invalid mul";
    }
}
/********************************************************************************/


using std::pair;
using std::make_pair;
using std::vector;
typedef std::complex<long double> complex_t;
struct AD5933{
  std::string firmware = "./AD5933_34FW.hex";
  AD5933();
  complex_t read_measurement();
  libusb_device_handle *h;
  libusb_context *ctx;
  double measure_temperature();
  int download_fx2();
  int read_register( uint8_t& buffer, uint8_t reg);
  int write_register( uint8_t command,uint8_t reg);
  long double clk;
  long double ext_clk=4000000l;
  long double int_clk=16776000l;
  uint8_t ctrl_reg1;
  uint8_t ctrl_reg2;
  uint8_t get_status();
  void choose_clock( Clk setting);
  void increase_frequency();
  void initilize_frequency();
  void power_down();
  void repeat_frequency();

  void print_command_registers();

  uint32_t get_frequency();
  
  void set_PGA ( Gain setting );
  void set_frequency_step ( uint32_t inc );
  void set_settling_cycles ( uint32_t cycles );
  void set_settling_multiplier (SettlingMultiplier setting);
  void set_standby();
  void set_starting_frequency ( uint32_t start );
  void set_step_number ( uint32_t number );
  void set_voltage_output ( Voltage setting );
  void start_sweep();

  std::string show_voltage();
  std::string show_mode();
  std::string show_gain();
  std::string show_clock();
  void print_device_state();
};

std::string AD5933::show_mode()
{
  uint8_t reg;
  this->read_register(reg,CTRL_MSB);
  reg &= MODE_MASK;
  Mode m;
  try
    {
      m = mode_map.at(reg);
    }
  catch (const std::out_of_range &e)
    {
      std::cout<<"Invalid mode value!!!";
      return "Invalid Mode";
    }
  switch (m)
    {
    case Mode::INIT_START_FREQ:
      {
	return "Frequency initialization";
      }
    case Mode::START_FREQ_SWEEP:
      {
	return "Start sweep";
      }
    case Mode::INC_FREQ:
      {
	return "Increase Frequency";
      }
    case Mode::REPEAT_FREQ:
      {
	return "Repeat Frequency";
      }
    case Mode::MEAS_TEMP:
      {
	return "Measuring Temperature";
      }
    case Mode::PD_MODE:
      {
	return "Powered Down";
      }
    case Mode::SB_MODE:
      {
	return "Standby";
      }
    default:
      {
	return "Invalid Mode";
      }
    } 
}


void AD5933::print_device_state()
{
  this->print_command_registers();
  std::stringstream s; 
  s<<"Device mode:\t"<<this->show_mode()<<"\n";
  s<<"Voltage Range:\t"<<this->show_voltage()<<"\n";
  s<<"PGA:\t"<<this->show_gain()<<"\n";
  s<<"Clock:\t"<<this->show_clock();
  std::cout<<s.str()<<std::endl;
}

std::string AD5933::show_voltage()
{
  uint8_t reg;
  this->read_register(reg,CTRL_MSB);
  reg &= VOLTAGE_MASK;
  Voltage v;
  try
    {
      v = voltage_map.at(reg);
    }
  catch (const std::out_of_range &e)
    {
      std::cout<<"Invalid Voltage Command!!!";
      return "Invalid Voltage";
    }
  switch (v)
    {
    case Voltage::OUTPUT_2Vpp:
      {
	return "2Vpp";
      }
    case Voltage::OUTPUT_200mVpp:
      {
	return "200mVpp";
      }
    case Voltage::OUTPUT_400mVpp:
      {
	return "400mVpp";
      }
    case Voltage::OUTPUT_1Vpp:
      {
	return "1Vpp";
      }
    default:
      {
	return "Invalid Voltage";
      }
    }
}

std::string AD5933::show_gain()
{
  uint8_t reg;
  this->read_register(reg,CTRL_MSB);
  reg &= PGA_MASK;
  Gain g;
  try
    {
      g = gain_map.at(reg);
    }
  catch (const std::out_of_range &e)
    {
      std::cout<<"Invalid gain!!!";
      return "Invalid gain";
    }
  switch (reg)
    {
    case PGA_GAIN5x:
      {
	return "5x";
      }
    case PGA_GAIN1x:
      {
	return "1x";
      }
    default:
      {
	return "Invalid Gain";
      }
    }
}

std::string AD5933::show_clock()
{
  uint8_t reg=0;
  this->read_register(reg, CTRL_LSB);
  reg &= CLK_MASK;
  if (reg)
    {
      return "External";
    }
  else
    {
      return "Internal";
    }
}


void AD5933::print_command_registers()
{
  uint8_t msb,lsb;
  read_register(msb, CTRL_MSB);
  read_register(lsb, CTRL_LSB);
  std::bitset<8> m(msb);
  std::bitset<8> l(lsb);
  for (int i=15;i>=0;i--)
    {
      printf("%d\t",i);
    }
  printf("\n");
  //  std::cout<<"15\t14\t13\t12\t11\t10\t9\t8\t7\t6\t5\t4\t3\t2\t1\t0\n";
  std::cout<<m[7]<<"\t"<<m[6]<<"\t"<<m[5]<<"\t"<<m[4]<<"\t"<<m[3]<<"\t"<<m[2]<<"\t"<<m[1]<<"\t"<<m[0]<<"\t"
	   <<l[7]<<"\t"<<l[6]<<"\t"<<l[5]<<"\t"<<l[4]<<"\t"<<l[3]<<"\t"<<l[2]<<"\t"<<l[1]<<"\t"<<l[0];
  printf("\n");
}



uint32_t AD5933::get_frequency()
{
  uint8_t r0,r1,r2;
  read_register(r0, FREQ_7_0);
  read_register(r1, FREQ_15_8);
  read_register(r2, FREQ_23_16);
  return  r2*1<<16 | r1*1<<8 | r0;
}

void AD5933::set_settling_multiplier(SettlingMultiplier setting)
{
  uint8_t settling_msb=0;
  read_register(settling_msb, SETTLE_MSB);
  settling_msb &= 0xF9; // Clear bit 1:2
  switch (setting)
    {
    case SettlingMultiplier::MUL_2x:
      settling_msb |= SETTLING_MUL_2x;
      break;
    case  SettlingMultiplier::MUL_4x:
      settling_msb |= SETTLING_MUL_4x;
      break;
    }
  write_register(settling_msb,SETTLE_MSB);
}

uint8_t AD5933::get_status()
{
  uint8_t buf=0;
  auto err = read_register(buf,SREG);
  if (err<0)
    {
      libusb_strerror(libusb_error(err));
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
  printf("start: %d ",start);
  uint8_t r2 = ( start & 0xff0000 ) >>16;
  uint8_t r1 = ( start & 0x00ff00 ) >>8;
  uint8_t r0 = ( start & 0x0000ff );
  printf("registers: 0x%X%X%X\n",r2,r1,r0);
  write_register ( r2, FREQ_23_16 );
  write_register ( r1, FREQ_15_8 );
  write_register ( r0, FREQ_7_0 );
}

void AD5933::set_frequency_step(uint32_t inc)
{
  uint8_t r2 = ( inc & 0xff0000 ) >>16;
  uint8_t r1 = ( inc & 0x00ff00 ) >>8;
  uint8_t r0 = ( inc & 0x0000ff );
  write_register ( r2, STEP_23_16 );
  write_register ( r1, STEP_15_8 );
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
  this->write_register(ctrl_reg2, CTRL_MSB);
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
  auto err = libusb_control_transfer ( h,0x40,0xDE,0x0D, command << 8 | reg,NULL,0,0 );
  if ( err<0 )
    {
      printf("Error writing 0x%X to register 0x%X",command,reg);
      const char *str = libusb_strerror( libusb_error( err ));
      fprintf(stderr,"%s\n",str);
    }
  uint8_t data=0;
  read_register(data,reg);
  if (command != data){
    printf("Invalid write!!!\n");
    printf("Wrote %d, got %d, on reg %d\n",command,data,reg);
  }
  return err;
}

int AD5933::read_register ( uint8_t &buffer, uint8_t reg )
{
  auto err = libusb_control_transfer ( h,0xc0,0xDE,0x0D,reg,&buffer,1,0 );
  if ( err<0 )
    {
      printf("Error reading from register 0x%X",reg);
      const char *str = libusb_strerror( libusb_error( err ));
      fprintf(stderr,"%s\n",str);
    }
  return err;
}

complex_t AD5933::read_measurement()
{
  uint8_t im1,im0,re1,re0;
  std::bitset<8> im1b,im0b,re1b,re0b;
  read_register(re1, REAL_MSB );
  re1b = re1;
  read_register(re0, REAL_LSB );
  re0b = re0;
  int16_t real = re1<<8 | re0 ;
  std::bitset<16> realb(real);
  
  read_register ( im1, IMG_MSB );
  im1b=im1;
  read_register ( im0, IMG_LSB );
  im0b=im0;
  int16_t img = im1<<8 | im0;
  std::bitset<16> imb(img);
  long double dreal = real;
  long double dimg = img;
  complex_t z(dreal,dimg);
#ifdef DEBUG
  std::cout<<"Real:\t\t\tImag:\n";
  std::cout<<re1b<<re0b<<"\t"<<im1b<<im0b<<"\n";
  std::cout<<realb<<"\t"<<imb<<"\n";
  std::cout<<dreal<<"\t\t\t"<<dimg<<"\n";
  std::cout<<z.real()<<"\t\t\t"<<z.imag()<<"\n";
#endif
  return z;
}


AD5933::AD5933()
{
  //  auto err = cyusb_open ( 0x0456, 0xb203 );
  h = NULL;
  ctx= NULL;
  auto err = libusb_init(&ctx);
  if (err)
  {
    fprintf(stderr,"Error in initializing libusb library...\n");
    const char *str = libusb_strerror( libusb_error( err ));
    fprintf(stderr,"%s\n",str);
    std::abort();
  }
  h = libusb_open_device_with_vid_pid(ctx, VID, PID);
  if (!h)
  {
    fprintf(stderr, "Device not found\n");
    std::abort();
  }
  
  err = libusb_kernel_driver_active ( h , 0 );
  if ( err != 0 )
  {
    fprintf (stderr, "Kernel driver active. Exitting\n" );
    libusb_close(h);
    libusb_exit(NULL);
    std::exit(-1);
  }
  err = libusb_claim_interface ( h, 0 );
  if ( err != 0 )
  {
    fprintf (stderr, "Error in claiming interface\n" );
    libusb_close(h);
    libusb_exit(NULL);
    std::exit(-1);
  }
  else
  {
    printf ( "Successfully claimed interface\n" );
  }
  
  struct stat statbuf;
  static char filename[] = "./AD5933_34FW.hex";
  err = stat ( filename, &statbuf );
  printf ( "File size = %d\n", ( int ) statbuf.st_size );
  sleep(1);

  auto extension = strtoul ( "0xA0", NULL, 16 );
  err = download_fx2 ();
  if ( err )
  {
    printf ( "Error downloading firmware: %d\n",err );
  }
  printf("reading ctrl\n");
  sleep(1);
  read_register(ctrl_reg1,CTRL_LSB);
  read_register(ctrl_reg2,CTRL_MSB);
  clk=int_clk;
  printf("done constr\n");
}

int AD5933::download_fx2()
{
  FILE *fp = NULL;
  char buf[256];
  char tbuf1[3];
  char tbuf2[5];
  char tbuf3[3];
  unsigned char reset = 0;
  int r;
  int count = 0;
  unsigned char num_bytes = 0;
  unsigned short address = 0;
  unsigned char *dbuf = NULL;
  int i;

  auto extension = strtoul ( "0xA0", NULL, 16 );
  unsigned char vendor_command=extension;

  fp = fopen(firmware.c_str(), "r" );
  tbuf1[2] ='\0';
  tbuf2[4] = '\0';
  tbuf3[2] = '\0';

  reset = 1;
  r = libusb_control_transfer(h, 0x40, 0xA0, 0xE600, 0x00, &reset, 0x01, 1000);
  if ( !r ) {
    printf("Error in control_transfer\n");
    return r;
   }
  sleep(1);

  count = 0;

  while ( fgets(buf, 256, fp) != NULL ) {
    if ( buf[8] == '1' )
      break;
    strncpy(tbuf1,buf+1,2);
    num_bytes = strtoul(tbuf1,NULL,16);
    strncpy(tbuf2,buf+3,4);
    address = strtoul(tbuf2,NULL,16);
    dbuf = (unsigned char *)malloc(num_bytes);
    for ( i = 0; i < num_bytes; ++i ) {
      strncpy(tbuf3,&buf[9+i*2],2);
      dbuf[i] = strtoul(tbuf3,NULL,16);
     }
    r = libusb_control_transfer(h, 0x40, vendor_command, address, 0x00, dbuf, num_bytes, 1000);
    if ( !r ) {
      printf("Error in control_transfer\n");
      free(dbuf);
      return r;
     }
    count += num_bytes;
    free(dbuf);
   }
  printf("Total bytes downloaded = %d\n", count);
  sleep(1);
  reset = 0;
  r = libusb_control_transfer(h, 0x40, 0xA0, 0xE600, 0x00, &reset, 0x01, 1000);
  fclose(fp);
  return 0;
}

std::vector<  std::pair<long double,  complex_t > > sweep_frequency ( uint32_t lower,uint32_t number_of_samples,long double step, AD5933* h )
{
  long double clk = h->clk;
  vector< pair<long double,complex_t>> measurements;
  long double lowerd= lower;
  auto startd = (lowerd / (clk/4))* (1<<27);
  auto incd = (step / (clk/4))*(1<<27);
  
  uint32_t start = startd;
  uint32_t inc = incd;
  h->set_starting_frequency ( start );
  h->set_frequency_step ( inc );
  h->set_step_number ( number_of_samples );
  
  h->set_standby();
  usleep ( 5e5 );

  h->initilize_frequency();
  usleep ( 5e5 );

  h->start_sweep();
  usleep( 5e5);

#ifdef DEBUG
  std::cout<<"clock "<<clk<<"\n";
  std::cout<<"lowerd:"<<lowerd<<" "<<lower<<std::endl;
  printf("lower %d start = 0x%X,inc = 0x%X\n",lower,start,inc);
  printf("Set frequency: 0x%X\n",h->get_frequency());
#endif
  
  /*Sweep loop*/
  long double true_freq;
  auto cur_freq = start;
  int i=0;
  uint8_t sreg;
  for ( ;; )
    {
      long double ar = ( cur_freq );
      true_freq = ar/ ( (1<<27)/(clk/4)) ;
      /*Read SREG for valid impedance meausurement*/
      uint8_t impedance_valid=0;
      std::bitset<8> sregB(h->get_status());
      //      std::cout<<sregB<< "sreg before\n";
      do
        {
	  sreg = h->get_status();
          impedance_valid = sreg & SREG_IMPED_VALID;
        }
      while ( !impedance_valid );
      sregB = h->get_status();
      //      std::cout<<sregB<<"sreg after"<<std::endl;
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
  else
    {
      auto frac = (f-g0.first)/(g1.first-g0.first);
      new_gain = (1-frac)*g0.second + frac*g1.second;
    }
  return new_gain;
}

template <typename T>
std::vector<std::pair<long double, long double>> calc_multigains(const std::vector<std::pair<long double, T>> &adm,
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

void write_to_file(const std::vector<std::pair<long double, long double>> &mag,
		   const std::vector<std::pair<long double, long double>> &phase,
		   const std::vector<std::pair<long double, complex_t>> &adm)
{
  if (mag.size()!=phase.size() || phase.size() !=adm.size())
    {
      fprintf(stderr, "write_to_file: Argument size not equal");
      std::abort();
    }
  std::string header = "Frequency,Impedance,Phase,Real,Imaginary,Magnitutude\n";
  FILE *fp =  fopen("output.csv","w");
  if (fp==NULL)
    {
      perror(NULL);
      return;
    }
  fprintf(fp, "%s", header.c_str());
  for (size_t i = 0; i<mag.size();++i)
    {
      auto err= fprintf(fp,"%Lf,%Lf,%Lf,%Lf,%Lf,%Lf\n",
			mag[i].first,mag[i].second,phase[i].second,
			adm[i].second.real(),adm[i].second.imag(),std::abs(adm[i].second));
      if (err<0)
	{
	  perror(NULL);
	  return;
	}
    }
  fclose(fp);
}
