#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#define MAX_FILE_NAME_SIZE 50
#define MAX_DATA_SIZE 63000

struct Request {
  uint8_t method_type;
  uint8_t message_type;
  uint16_t data_size;
  uint8_t file_type;
  uint8_t filename_size;
  char file_name[MAX_FILE_NAME_SIZE];
  char data[MAX_DATA_SIZE];
};

#define Message 1
#define File 2

#define Post 1
#define Get 2

#define None 0
#define C 1
#define Go 2
#define Js 3
#define Ts 4
#define Vue 5

#endif

