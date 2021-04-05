/*
The MIT License
Copyright (c) 2020-2027 Isamu.Yamauchi , 2019.5.13 Update 2021.4.5
pepoambme680.c read bme680 temperature,humidity,presure,gas

Download bme680.c bme680.h bme680_defs.h from https://github.com/BoschSensortec/BME680_driver
cc pepobme680.c bme680.c -o pepobme680
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>          /* for O_RDWR */
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>  /* for I2C_SLAVE */
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>         /* use signal handler */
#define I2C_DEVICE "/dev/i2c-1"
#define DELAY 3000          /* for measurement delay ms */
#define LOOP_TIME 5000      /* senser read period ms */
#define DESTZONE "TZ=Asia/Tokyo"  /* destination time zone */
#define SENSOR_DATA "/www/remote-hand/tmp/.pepobme680"  /* read sensor dta file */
#define SENSOR_DATA_TMP "/www/remote-hand/tmp/.pepobme680_tmp"  /* read sensor file data temporary */

#include "bme680.h"
/* BME680 I2C addresses defined bme680_def.h But can be changed here */
#ifdef BME680_I2C_ADDR_SECONDARY
#undef BME680_I2C_ADDR_SECONDARY
#endif
#define BME680_I2C_ADDR_SECONDARY  UINT8_C(0x76)
struct bme680_dev gas_sensor;
struct bme680_field_data data;
int i2c_fd;
FILE *data_fd;
uint16_t meas_period;

void user_delay_ms(uint32_t period)
{
  struct timeval timeout;
  timeout.tv_sec = period / 1000;
  timeout.tv_usec = (period % 1000) * 1000;
  if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
    perror("user_delay_ms");
  }
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  uint8_t reg[1];
  reg[0]=reg_addr;
   if (write(i2c_fd, reg, 1) != 1) {
    perror("user_i2c_read_reg");
    rslt = 1;
  }
  if (read(i2c_fd, reg_data, len) != len) {
    perror("user_i2c_read_data");
    rslt = 1;
  }
  return rslt;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  uint8_t reg[16];
  reg[0]=reg_addr;
  for (int i=1; i<len+1; i++)
     reg[i] = reg_data[i-1];
  if (write(i2c_fd, reg, len+1) != len+1) {
    perror("user_i2c_write");
    rslt = 1;
    exit(1);
  }
  return rslt;
}

void conf_bme680()
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  uint8_t set_required_settings;
  rslt = ioctl( i2c_fd, I2C_SLAVE, BME680_I2C_ADDR_SECONDARY );
  // init device
  //  set address of i2c_BME680
  gas_sensor.dev_id = BME680_I2C_ADDR_SECONDARY;
  gas_sensor.intf = BME680_I2C_INTF;
  gas_sensor.read = user_i2c_read;
  gas_sensor.write = user_i2c_write;
  gas_sensor.delay_ms = user_delay_ms;
  rslt = BME680_OK;
  rslt = bme680_init(&gas_sensor);
  /* Set the temperature, pressure and humidity settings */
  gas_sensor.tph_sett.os_hum = BME680_OS_2X;
  gas_sensor.tph_sett.os_pres = BME680_OS_4X;
  gas_sensor.tph_sett.os_temp = BME680_OS_8X;
  gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;
  /* Set the remaining gas sensor settings and link the heating profile */
  gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
  /* Create a ramp heat waveform in 3 steps */
  gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
  gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */
  /* Select the power mode */
  /* Must be set before writing the sensor configuration */
  gas_sensor.power_mode = BME680_FORCED_MODE;
  /* Set the required sensor settings needed */
  set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL
    | BME680_GAS_SENSOR_SEL;
  /* Set the desired sensor configuration */
  rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);
  /* Set the power mode */
  rslt = bme680_set_sensor_mode(&gas_sensor);
  /* Get the total measurement duration so as to sleep or wait till the
   * measurement is complete */
  bme680_get_profile_dur(&meas_period, &gas_sensor);
  user_delay_ms(meas_period + DELAY); /* Delay till the measurement is ready */
}
void close_fd()
{
  close(i2c_fd);
  unlink(SENSOR_DATA);
  unlink(SENSOR_DATA_TMP);
  exit(1);
}

void move_file(const char* src_name, const char* dest_name)
{
  rename(src_name, dest_name);
}

int main(int argc, char *argv[] )
{
  signal(SIGTERM,close_fd);
  signal(SIGQUIT,close_fd);
  signal(SIGINT,close_fd);
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  putenv(DESTZONE);               // Switch to destination time zone
  i2c_fd = open(I2C_DEVICE, O_RDWR);
  if (i2c_fd < 0) {
    perror("bme680 open");
    exit(-1);
  }
  conf_bme680();
  // Get sensor data
  // Avoid using measurements from an unstable heating setup
  while(1)
    {
      rslt = bme680_get_sensor_data(&data, &gas_sensor);
      if(data.status & BME680_HEAT_STAB_MSK)
      {
        data_fd = fopen(SENSOR_DATA_TMP,"w");
        if(data_fd < 0){
          exit(-1);
        }
      t = time(NULL);
      tm = *localtime(&t);
      fprintf(data_fd,"%d/%02d/%02d/%02d:%02d:%02d,", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
      fprintf(data_fd,"%.1f,%.1f,%.1f,%d", data.temperature / 100.0f, data.humidity / 1000.1f,
        data.pressure / 100.0f, data.gas_resistance);
      fclose(data_fd);
      move_file(SENSOR_DATA_TMP,SENSOR_DATA);
    }
    // Trigger a meausurement
    rslt = bme680_set_sensor_mode(&gas_sensor); /* Trigger a measurement */
    // Wait for a measurement to complete
    user_delay_ms(meas_period + DELAY); /* Wait for the measurement to complete */
    user_delay_ms(LOOP_TIME);
  }
  return 0;
}
