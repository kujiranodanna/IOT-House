/*
The MIT License
Copyright (c) 2020-2027 Isamu.Yamauchi , 2021.6.6 Update 2021.6.28
pepotocosbme680.c reads temperature, humidity, pressure and gas from BME680 via tocos

Download bme680.c bme680.h bme680_defs.h from https://github.com/BoschSensortec/BME680_driver
cc pepotocosbme680.c bme680.c -o pepobme680
o 2021.6.28 ver 0.1
  1st release
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>  /* for O_RDWR */
#include <sys/ioctl.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>  /* use signal handler */
#include <string.h>
#include <stdint.h>
#include "hidapi/hidapi.h"
#include <sys/types.h>
#include <sys/sem.h>
#ifdef __STDC__
#define sigtype void
#else
#define sigtype int
#endif
#define DEBUG
#undef  DEBUG /* Comment out the case when debugging */
#define DEMO
#undef  DEMO /* Comment out the case when DEMO */
#define READ 'R'
#define WRITE 'W'
#define VER "0.4"
#define DAY "compiled:"__DATE__
#define LOCK -1
#define UNLOCK 1
#define TOCOS_SEMAPHORE "/var/run/pepotocosbme680.semaphore"
#ifdef  DEBUG
#define DEBUG_WAIT 10000 /* debug wait timer, ms */
#endif
#define AM2320_SLAVE_ADDR 0x5C
#define CP2112_RESET 0x01
#define CP2112_GETSET_GPIO_CONFIG 0x02
#define CP2112_GET_GPIO 0x03
#define CP2112_SET_GPIO 0x04
#define CP2112_GETSET_SMBUS_CONFIG 0x06
#define CP2112_DATA_READ_REQ 0x10
#define CP2112_DATA_WRITE_READ_REQ 0x11
#define CP2112_DATA_READ_FORCE_SEND 0x12
#define CP2112_DATA_READ_RESPONS 0x13
#define CP2112_DATA_WRITE 0x14
#define CP2112_TRANSFER_STATUS_REQ 0x15
#define CP2112_TRANSFER_STATUS_RESP 0x16
#define CP2112_TRANSFER_CANCEL 0x17
#define CP2112_VID 0x10c4
#define CP2112_PID 0xea90
#define CP2112_IS_OPEN 0x01
#define CP2112_IS_CLOSE 0x00
#define DELAY 3000  /* for measurement delay ms */
#define LOOP_TIME 5000  /* senser read period ms */
#define HID_WAIT 5  /* for hid delay ms */
#define DESTZONE    "TZ=Asia/Tokyo"  /* destination time zone */
#define SENSOR_DATA "/www/remote-hand/tmp/.pepotocosbme680"  /* read sensor dta file */
#define SENSOR_DATA_TMP "/www/remote-hand/tmp/.pepotocosbme680_tmp"  /* read sensor file data temporary */
#define I2COK "01"
#define I2CNG "00"
#define I2C_WAIT 50
#define INBUFF 256

#include "bme680.h"
/* BME680 I2C addresses defined bme680_def.h But can be changed here */
#define BME680_I2C_ADDR_PRIMARY    UINT8_C(0x76)
#define BME680_I2C_ADDR_SECONDARY  UINT8_C(0x77)
struct bme680_dev gas_sensor;
struct bme680_field_data data;
FILE *data_fd;
uint16_t meas_period;
int mysem_id = 0;
hid_device *hd;
int8_t is_hid = CP2112_IS_CLOSE;

void user_delay_ms(uint32_t period)
{
  struct timeval timeout;
  timeout.tv_sec = period / 1000;
  timeout.tv_usec = (period % 1000) * 1000;
  if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
  {
    perror("user_delay_ms");
  }
}

void usage()
{
  fprintf(stderr,"\r\n** Welcome to pepotocosbme680 Version-%s Copyright Isamu.Yamauchi %s **",VER,DAY);
  fprintf(stderr,"\n\rusage:pepotocosbme680 port:0-8 [0|1] [timer:0-300000ms]");
  fprintf(stderr,"\n\rusage:pepotocosbme680 port:0-3 output, 4-7 input ");
  fprintf(stderr,"\n\rusage:pepotocosbme680 5  <--AM2320 measured");
  fprintf(stderr,"\n\rusage:pepotocosbme680 10  <--BME680 measured\n\r");
}

struct bme680_dev gas_sensor;
struct bme680_field_data data;
FILE *data_fd;
FILE *cmd_fd;
uint16_t meas_period;
int8_t is_i2c_fd = I2C_FD_IS_CLOSE;

int get_myval(int sid)
{
  union semun
  {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
    void *__pad;
  };
  union semun my_semun;
  uint16_t d_result = semctl(sid, 0, GETVAL, my_semun);
  if (d_result == -1)
  {
    perror("semctl: GETVAL failed");
    exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  printf("%s%d\n","val=",d_result);
#endif
  return(d_result);
}

int get_sempid(int sid)
{
  union semun
  {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
    void *__pad;
  };
  union semun my_semun;
  pid_t sem_pid;
  sem_pid = semctl(sid, 0, GETPID, my_semun);
  if (sem_pid == -1)
  {
    perror("semctl: GETPID failed");
    exit(EXIT_FAILURE);
  }
  return(sem_pid);
}

void create_semaphore()
{
  union semun
  {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
    void *__pad;
  };
  union semun my_semun;
  FILE *fdsem;
  uint16_t d_result;
  int mysemun_id;
  key_t key;
#ifdef DEBUG
  pid_t my_pid, sem_pid;
  my_pid = getpid();
#endif
  fdsem = fopen(TOCOS_SEMAPHORE,"r");
  if (fdsem != NULL)
  {
    if ((key = ftok(TOCOS_SEMAPHORE, 'S')) == -1)
    {
      perror("ftok: failed");
      exit(EXIT_FAILURE);
    }
/* Creating of the semaphore */
    mysemun_id = semget(key, 1, 0666 | IPC_CREAT);
    if (mysemun_id == -1)
    {
      perror("semget: semget get failed");
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    sem_pid = get_sempid(mysemun_id);
    fprintf(stderr,"\r\nmy_pid:%d, sem_pid:%d\r\n",my_pid, sem_pid);
#endif
/* remove of the semaphore */
    my_semun.val = 1;
    if (semctl(mysemun_id , 0, IPC_RMID, my_semun) == -1)
    {
      perror("semctl: semaphore remove failed");
      exit(EXIT_FAILURE);
    }
/* semaphore of the file delete */
    unlink(TOCOS_SEMAPHORE);
  }
  fdsem = fopen(TOCOS_SEMAPHORE,"r");
  if (fdsem == NULL)
  {
/* File creation of semaphore */
    fdsem = fopen(TOCOS_SEMAPHORE,"w");
    if (fdsem == NULL)
    {
      perror("fopen: failed");
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    fprintf(stderr,"\r\n** %s file creation succeed of semaphore! **\r\n",TOCOS_SEMAPHORE);
#endif
    fclose(fdsem);
  }
  if ((key = ftok(TOCOS_SEMAPHORE, 'S')) == -1)
  {
    perror("ftok: failed");
    exit(EXIT_FAILURE);
  }
/* Creating of the semaphore */
  mysemun_id = semget(key, 1, 0666 | IPC_CREAT);
  if (mysemun_id == -1)
  {
    perror("semget: semget Initialization failed");
    exit(EXIT_FAILURE);
  }
  d_result = get_myval(mysemun_id);
  if (d_result == 0)
  {
/* Initialization of the semaphore */
    my_semun.val = 1;
    if (semctl(mysemun_id, 0, SETVAL, my_semun) == -1)
    {
      perror("semctl: Initialization failed");
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
  fprintf(stderr,"\r\n** Initialization succeed of semaphore! **\r\n");
#endif
  }
}
void mysem_lock(int sid)
{
  key_t key;
  FILE *fdsem;
  if (sid == 0)
  {
    fdsem = fopen(TOCOS_SEMAPHORE,"r");
    if (fdsem == NULL)
    {
      create_semaphore();
      cp2112_open(CP2112_VID, CP2112_PID);
      cp2112_config_gpio(hd);
    // DEMO
    #ifdef DEMO
      unsigned char mask = 0x7f;
      unsigned char value = 0x7f;
    #else
      unsigned char mask = 0x0f;
      unsigned char value = 0x00;
    #endif
      cp2112_set_gpio(hd, mask, value);
      if (cp2112_set_auto_send_read(hd, 0) < 0)
      {
        fprintf(stderr, "set_auto_send_read failed\n");
        is_hid = CP2112_IS_CLOSE;
        raise(SIGTERM);
      }
    }
    else
      fclose(fdsem);
    if ((key = ftok(TOCOS_SEMAPHORE, 'S')) == -1)
    {
      perror("ftok: failed");
      exit(EXIT_FAILURE);
    }
    if (mysem_id == 0)
    {
  /* Initialization of the semaphore */
      mysem_id = semget(key, 1, 0666 | IPC_CREAT);
    }
    if (mysem_id  < 0)
    {
      perror("semget: semget Initialization failed");
      exit(EXIT_FAILURE);
    }
    sid = mysem_id;
  }
  if (is_hid == CP2112_IS_CLOSE) cp2112_open(CP2112_VID, CP2112_PID);
  struct sembuf mysemop[1];
  mysemop[0].sem_num = 0;
  mysemop[0].sem_op = LOCK;
  mysemop[0].sem_flg = SEM_UNDO;
  if (semop(sid, mysemop, 1) == -1)
  {
    perror("semop: semop lock-1 failed");
    exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  printf("semop_lock:");get_myval(sid);
#endif
}

void mysem_unlock(int sid)
{
  struct sembuf mysemop[1];
  mysemop[0].sem_num = 0;
  mysemop[0].sem_op = UNLOCK;
  mysemop[0].sem_flg = SEM_UNDO;
  if (semop(sid, mysemop, 1) == -1)
  {
    perror("semop: semop unlock failed");
    exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  printf("sem_unlock:");get_myval(sid);
#endif
}

sigtype close_fd()
{
  mysem_unlock(mysem_id);
  unlink(SENSOR_DATA);
  unlink(SENSOR_DATA_TMP);
  if (is_hid == CP2112_IS_OPEN)
  {
    hid_close(hd);
    hid_exit();
  }
  exit(EXIT_SUCCESS);
}

void move_file(const char* src_name, const char* dest_name)
{
  rename(src_name, dest_name);
}

/* CRC16 calculation */
unsigned short crc16( unsigned char *ptr, unsigned char len ) {
  unsigned short crc = 0xFFFF;
  unsigned char i;
  while( len-- )
  {
    crc ^= *ptr++;
    for(i = 0; i < 8; i++)
    {
      if(crc & 0x01)
      {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

int cmd_fd_open()
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  int8_t retry = 0;
  srand((unsigned int)time(NULL));
  pid_t my_pid = 0;
  int old_pid = 0;
  FILE *lock_fd;
  uint32_t time_wait = 0;
  my_pid = getpid();
  while (1)
  {
    lock_fd = fopen(LOCK_FILE,"r");
    if (lock_fd < 0)
    {
      lock_fd = fopen(LOCK_FILE,"w");
      fprintf(lock_fd,"%d",my_pid);
      close(lock_fd);
      cmd_fd = fopen(CMD_FILE,"w");
      if (cmd_fd < 0)
      {
        rslt = -1;
        return rslt;
      } else {
        rslt = 0;
        return rslt;
      }
      break;
    } else {
      fscanf(lock_fd,"%d",&old_pid)
      if (old_pid == my_pid)
      {
        close(lock_fd);
        unlink(LOCK_FILE);
      } else {
        retry++;
        if (retry > 3)
        {
          return(-1);
        }
        sprintf(time_wait,"%2d",rand());
        user_delay_ms(time_wait);
        continue;
      }
    }
  }
}

int char2hex(unsigned char ch)
{
  if ( ch >= '0' && ch <= '9' )
    return (ch - '0') ;
  else if ( ch >= 'a' && ch <= 'f' )
    return (ch - 'a' + 10) ;
  else if ( ch >= 'A' && ch <= 'F' )
    return (ch - 'A' + 10) ;
}

int checksum(unsigned char *ch) {
  int i = 0 ;
  int j = 0 ;
  int len = 0 ;
  unsigned char s[3] = { 0 };
  unsigned char result = 0x00 ;
  len = strlen(ch) ;
  for ( i = 0 ; i < len - 2 ; i = i + 2 )  {
    strncpy( s , ch + i , 1 ) ;
    s[1] = '\0' ;
    result = result + (char2hex(*s) * 16) ;
	j = i + 1;
    strncpy( s , ch + j , 1 ) ;
    s[1] = '\0' ;
    result = result + char2hex(*s) ;
  }
  result = ( result ^ 0xff ) + 1 ;
  sprintf( s , "%02x" , result );
  printf("%c%c\n", toupper(s[0]) , toupper(s[1]) ) ;
}

int cmd_exec()
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  FILE	*fp;
  unsigned char cmd_line[256] = { 0 };
  unsigned cahr buf[256] = { 0 };
  close(cmd_fd);
  sprintf(cmd_line,"%s%s",CMD,CMD_FILE);
  if ((fp = popen(cmdline,"r")) == NULL)
  {
    perror ("can not exec commad");
    rslt = -1;
    unlink CMD_FILE;
    return NULL;
  } else {
    while (!feof(fp))
    {
      fgets(buf, sizeof(buf), fp);
    }
    (void) pclose(fp);
    unlink CMD_FILE;
    return *fp;
  }
}

void close_fd()
{
  i2c_fd_close(i2c_fd);
  unlink(SENSOR_DATA);
  unlink(SENSOR_DATA_TMP);
  exit(EXIT_SUCCESS);
}

const char *twlite_am2320_read = "#!/bin/bash\n"
  "RETRY=5\n"
  "I2CRD=\"-1\"\n"
  "while [ ${RETRY} -ne 0 ];do\n"
    "retry_time=`echo -en $RANDOM |cut -c 1-2`\n"
    "echo -en \":7888AA015C0000X\r\n"\n"
    "msleep 50\n"
    "read -s -t 1 I2CRD || I2CRD=\"-1\"\n"
    "echo -en \":7888AA015C03020004X\r\n"\n"
    "msleep 50\n"
    "read -s -t 1 I2CRD || I2CRD=\"-1\"\n"
    "msleep\n"
    "echo -en \":7888AA025C0006X\r\n\"\n"
    "msleep 50\n"
    "read -s -n 28 -t 1 I2CRD || I2CRD=\"-1\"\n"
    "TMP=`echo -en ${I2CRD} | wc -c`\n"
    "[ ${TMP} -eq 28 ] && break\n"
    "RETRY=$((${RETRY} - 1))\n"
    "[ ${RETRY} -eq 0 ] && break\n"
    "RETRY=$((${RETRY} - 1))\n"
    "msleep $retry_time\n"
    "I2CRD=\"-1\"\n"
  "done\n"
    "echo -en ${I2CRD} >/dev/stderr\n"
;

const char *twlite_di_read = "#!/bin/bash\n"
"RETRY=5\n"
"while [ ${RETRY} -ne 0 ];do\n"
"  retry_time=`echo -en $RANDOM |cut -c 1-2`\n"
"  echo -en \":78800100080000000000000000X\r\n"\n"
"  msleep 50\n"
"  echo -en \":78800108080000000000000000X\r\n\"\n"
"  msleep 50\n"
"  read -s -n 49 -t 1 RD || RD=\"-1\""
"  echo -en \":78800100080000000000000000X\r\n\"\n"
"  TMP=`echo -en ${RD} | wc -c`\n"
"  [ ${TMP} -eq 49 ] && break\n"
"  RETRY=$((${RETRY} - 1))\n"
"  msleep $retry_time\n"
"  RD=\"-1\"\n"
"done\n"
"  echo -en ${RD} >/dev/stderr\n"
"}\n"
;

const char *twlite_do_write = "#!/bin/bash\n"
"RETRY=5\n"
"while [ ${RETRY} -ne 0 ];do\n"
  "retry_time=`echo -en $RANDOM |cut -c 1-2`\n"
  "echo -en \":7880010F0F0000000000000000X\r\n\"\n"
  "msleep 50\n"
  "echo -en \":78800108080000000000000000X\r\n\"\n"
  "msleep 50\n"
  "read -s -n 49 -t 1 RD || RD=\"-1\"\n"
  "echo -en \":78800100080000000000000000X\r\n\"\n"
  "TMP=`echo -en ${RD} | wc -c`\n"
  "[ ${TMP} -eq 49 ] && break\n"
  "RETRY=$((${RETRY} - 1))\n"
  "msleep $retry_time\n"
  "RD=\"-1\"\n"
"done\n"
  "echo -en ${RD} >/dev/stderr\n"
;

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  uint8_t reg[1] = { 0 };;
  uint8_t rbyte = len * 2;
  uint8_t sbyte = 1 * 2;
  reg[0]=reg_addr;
  rslt = cmd_fd_open();
  if (rslt < 0)
  {
    exit(EXIT_FAILURE);
  } else {
    fprintf(cmd_fd,"\

  if (write(i2c_fd, reg, 1) != 1) {
    perror("user_i2c_read_reg");
    i2c_fd_close();
    rslt = 1;
  }
  if (read(i2c_fd, reg_data, len) != len) {
    perror("user_i2c_read_data");
    i2c_fd_close();
    rslt = 1;
  }
  i2c_fd_close();
  return rslt;
}

int user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  int fp;
  uint8_t reg[16] = { 0 };
  uint8_t sbyte = len * 2;
  reg[0]=reg_addr;
  rslt = cmd_fd_open();
  if (rslt < 0)
  {
    exit(EXIT_FAILURE);
  } else {
    fprintf(cmd_fd,"
    fp = cmd_exec();
    if (fp == NULL)
    {
      return -1;
    }
    return 0;
  }
}

/* CRC16 calculation */
unsigned short crc16( unsigned char *ptr, unsigned char len ) {
  unsigned short crc = 0xFFFF;
  unsigned char i;
  while( len-- )
  {
    crc ^= *ptr++;
    for(i = 0; i < 8; i++)
    {
      if(crc & 0x01)
      {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

int am2320_measured(hid_device *hd)
{
  int i = 0;
  int ret = -1;
  int retry_cnt = 0;
  int timeout = 1000; // 1000 milisecons
  unsigned short crc_m, crc_s;
  unsigned char crc_tmp[6];
  unsigned char buf_in[12] = { 0 };
  unsigned char buf_out[12] = { 0 };
  buf_out[0] = CP2112_DATA_WRITE;
/* 0x5c is the 7-bit SMBus slave address of the am2320 */
  buf_out[1] = AM2320_SLAVE_ADDR<<1;
/* These three steps can be completed by the sensor and writes reads
https://www.silabs.com/community/interface/knowledge-base.entry.html/2014/10/21/cp2112_returns_incor-Dbhn
*/
  while (retry_cnt < 5)
  {
    ret = cp2112_is_idle(hd);
 // ret: 0x01 is Busy
    if (ret == 1)
    {
      retry_cnt++;
      user_delay_ms(HID_WAIT);
      continue;
    }
// am2320 wake up
    ret = hid_write(hd, buf_out, 2);
// ret: 0x02 is Complete
    if (ret != 2)
    {
      retry_cnt++;
      user_delay_ms(HID_WAIT);
      continue;
    }
    user_delay_ms(HID_WAIT);
/* write a buf_out byte to the am2320 */
    buf_out[0] = CP2112_DATA_WRITE;
    buf_out[2] = 0x03; // writes length
    buf_out[3] = 0x03; // measured am2320 specification 1
    buf_out[4] = 0x00; // specification 2
    buf_out[5] = 0x04; // specification 3
    ret = hid_send_feature_report(hd, buf_out, 6);
    if (ret != 6)
    {
      retry_cnt++;
      continue;
    }
    user_delay_ms (HID_WAIT);
    buf_out[0] = CP2112_DATA_READ_REQ;
    buf_out[2] = 0x00; // reads length_high
    buf_out[3] = 8; // reads length_low
    ret = hid_send_feature_report(hd, buf_out, 4);
    if (ret < 0)
    {
      fprintf(stderr, "hid_write() failed: %ls\n" ,
      hid_error(hd));
     return -1;
    }
    user_delay_ms (HID_WAIT);
    buf_out[0] = CP2112_DATA_READ_FORCE_SEND;
    buf_out[2] = 0x00; // reads length_high
    buf_out[3] = 8; // reads length_low
    ret = hid_send_feature_report(hd, buf_out, 4);
    if (ret < 0)
    {
      fprintf(stderr, "hid_write() failed: %ls\n" ,
      hid_error(hd));
      return -1;
    }
    user_delay_ms(HID_WAIT);
    ret = hid_read_timeout(hd, buf_in, 11, timeout);
/*
    fprintf(stderr, "\nret: %d retry_cnt: %d\n",ret,retry_cnt);
    fprintf(stderr, "\nbuf_in dump start\n");

    for (i = 0; i < sizeof(buf_in); i++)
      {
        fprintf(stderr, "%d:%02x ",i ,buf_in[i]);
      }
    fprintf(stderr, "\nbuf_in dump end\n");
*/
    if (ret < 0)
    {
      fprintf(stderr, "hid_read() failed: %ls\n",
      hid_error(hd));
      return -1;
    }
    if (buf_in[0] == CP2112_DATA_READ_RESPONS && buf_in[1] == 0x02)
      break;
  }
/* Dummy reading */
  ret = cp2112_is_idle(hd);
  if ( retry_cnt > 5 )
  {
    fprintf(stderr, "-1");
    return -1;
  }
  if (buf_in[2] != 0x08)
  {
    /* read length not match */
    fprintf(stderr, "-1");
    return -1;
  }
/*
buf_in[0]:hid_report ID->0x13:CP2112_DATA_READ_RESPONS
buf_in[1]:hid_status 0x00->Idle ,x01->Busy ,x02->Complete
,buf_in[2]:hid_read_length
Following_AM2320_Read_Data
buf_in[3]:Function Code ,buf_in[4]:data length ,buf_in[5]:high humidity ,buf_in[6]:low humidity ,
buf_in[7]:high temperature ,buf_in[8]:low temperature ,buf_in[9]:CRC checksum low byte ,
buf_in[10]:CRC checksum low byte
0x03(Function Code)+0x04(data length)+0x03(high humidity)+0x39(low humidity) +
0x01 (high temperature) +0x15(low temperature)+0xE1(CRC checksum low byte) + 0xFE
(CRC checksum high byte);
Therefore: 0339H = 3×256 +3×16 +9 = 825 => humidity = 825÷10 = 82.5% RH;
0115H = 1×256 +1×16 +5 = 277 => temperature = 277÷10 = 27.7 ℃
*/
  for (i = 0; i < sizeof(crc_tmp); i++)
    crc_tmp[i] = buf_in[i + 3];
  crc_m = crc16(crc_tmp, sizeof(crc_tmp));
  crc_s = (buf_in[10] << 8) + buf_in[9];
  if (crc_m != crc_s)
  {
    return -1;
  } else {
    int humidity = buf_in[5] * 256 + buf_in[6];
    int humidity_high = humidity / 10;
    int humidity_low = humidity % 10;
    int temperature = buf_in[7] * 256 + buf_in[8];
    int temperature_high = temperature / 10;
    int temperature_low = temperature % 10;
    fprintf(stderr ,"%d.%d %d.%d",temperature_high ,temperature_low ,humidity_high ,humidity_low);
    return 0;
  }
}

int conf_bme680()
{
  int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
  uint8_t set_required_settings;
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
  /* Delay till the measurement is ready */
  user_delay_ms(meas_period + DELAY); /* Delay till the measurement is ready */
  return rslt;
}

int bme680_measured(hid_device *hd)
{
  int8_t rslt = 0;  /* Return 0 for Success, non-zero for failure */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  putenv(DESTZONE);  // Switch to destination time zone
  if (bme680_is_connect(hd) < 0)
  {
    fprintf(stderr, "bme680_measured() bme680_is_connect() failed: %ls\n" ,hid_error(hd));
    raise(SIGTERM);
  }
  mysem_unlock(mysem_id);
/* Get sensor data Avoid using measurements from an unstable heating setup */
  if (conf_bme680(hd) != 0)
  {
    fprintf(stderr, "bme680_measured() conf_bme680() failed: %ls\n" ,hid_error(hd));
    raise(SIGTERM);
  }
  while(1)
  {
    rslt = bme680_get_sensor_data(&data, &gas_sensor);
    if(rslt != 0)
    {
      conf_bme680(hd);
      continue;
    }
    if(data.status & BME680_HEAT_STAB_MSK)
    {
      data_fd = fopen(SENSOR_DATA_TMP,"w");
      if(data_fd < 0)
      {
        raise(SIGTERM);
      }
      t = time(NULL);
      tm = *localtime(&t);
      fprintf(data_fd,"%d/%02d/%02d/%02d:%02d:%02d,", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
      fprintf(data_fd,"%.1f,%.1f,%.1f,%d", data.temperature / 100.0f, data.humidity / 1000.1f,data.pressure / 100.0f, data.gas_resistance);
      fclose(data_fd);
      move_file(SENSOR_DATA_TMP,SENSOR_DATA);
    }
/* Trigger a meausurement */
    rslt = bme680_set_sensor_mode(&gas_sensor);
/* Wait for a measurement to complete */
    user_delay_ms(meas_period + DELAY);
/* Wait for the Measurement interval */
    user_delay_ms(LOOP_TIME);
  }
  return rslt;
}

int main(int argc, char *argv[])
{
  signal(SIGTERM,close_fd);
  signal(SIGQUIT,close_fd);
  signal(SIGINT,close_fd);
  signal(SIGHUP,close_fd);
  signal(SIGFPE,SIG_IGN);
  int port = 0;
  int data = 0;
  int invert_data = 0;
  int port_timer = 0;
  char rw_flag = READ;
  unsigned char s_result = 0x00;
  char patterns[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
  if ( argc > 4 || argc < 2  )
  {
    usage();
    exit(EXIT_FAILURE);
  }
  else {
    port = atoi(argv[1]);
    if (port > 10 || port < 0)
    {
      usage();
      exit(EXIT_FAILURE);
    }
  }
  if ( argc == 3 ||  argc == 4)
  {
    data = atoi(argv[2]);
    if ( data != 0 && data != 1 )
    {
      usage();
      exit(EXIT_FAILURE);
    }
    else {
      rw_flag = WRITE;
    }
  }
  if ( argc == 4 )
  {
    port_timer = atoi(argv[3]);
    if ( port_timer < 0 || port_timer > 300000 )
    {
      usage();
      exit(EXIT_FAILURE);
    }
    rw_flag = WRITE;
  }
  mysem_lock(mysem_id);
  if (rw_flag == WRITE)
  {
  /* port write */
    gpio_write(hd, port, data);
    if (port_timer > 0)
    {
      mysem_unlock(mysem_id);
      if (data == 0)
      {
        invert_data = 1;
      }
      else {
        invert_data = 0;
      }
      user_delay_ms(port_timer);
      mysem_lock(mysem_id);
      gpio_write(hd, port, invert_data);
    }
    s_result = gpio_read(hd) & patterns[port];
    s_result = s_result >> port;
    fprintf(stderr,"%1x",s_result);
  }
  if (rw_flag == READ)
  {
  /* port read */
#ifdef DEMO
    if (port == 9)
    {
      am2320_measured(hd);
    }
    else if (port == 10)
    {
  /* cp2112 reset */
      cp2112_reset(hd);
      mysem_unlock(mysem_id);
      unlink(TOCOS_SEMAPHORE);
      exit(EXIT_SUCCESS);
    }
#else
    if (port == 5)
    {
      am2320_measured(hd);
    }
    else if (port == 9)
    {
  /* cp2112 reset */
      cp2112_reset(hd);
      cp2112_close(hd);
      mysem_unlock(mysem_id);
      unlink(TOCOS_SEMAPHORE);
      exit(EXIT_SUCCESS);
    }
#endif
    else if (port == 8)
    {
      s_result = gpio_read(hd) & 0xff;
      fprintf(stderr,"%02x",s_result);
    }
    else if (port == 10)
    {
      bme680_measured(hd);
    }
    else
    {
      s_result = gpio_read(hd) & patterns[port];
      s_result = s_result >> port;
      fprintf(stderr,"%1x",s_result);
    }
  }
  mysem_unlock(mysem_id);
#ifdef DEBUG
  mysem_lock(mysem_id);
  fprintf(stderr,"lock & wait %d milliseconds\n",DEBUG_WAIT);
  user_delay_ms(DEBUG_WAIT);
  mysem_unlock(mysem_id);
#endif
  cp2112_close(hd);
  exit(EXIT_SUCCESS);
}
