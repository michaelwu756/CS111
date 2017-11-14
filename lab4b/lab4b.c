#include<mraa/aio.h>
#include<mraa/types.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
const int B = 4275;
const int R0 = 100000;

void sig_handler(int sig)
{
    fprintf(stderr, "Caught sigint signal number %d\n", sig);
    exit(1);  
}

int main()
{
  mraa_aio_context adc_a0;
  uint16_t adc_value = 0;
  float adc_value_float = 0.0;
  mraa_result_t r = MRAA_SUCCESS;
  int i=0;
  adc_a0 = mraa_aio_init(i);
  while(adc_a0==NULL)
  {
    fprintf(stderr, "Nothing connected to a%d\n", i);
    i++;
    adc_a0= mraa_aio_init(i);
  }
  fprintf(stderr, "Connected to a%d\n", i);

  signal(SIGINT, sig_handler);
  int running = 0;
  while (running == 0) {
    adc_value = mraa_aio_read(adc_a0);
    adc_value_float = mraa_aio_read_float(adc_a0);
    fprintf(stdout, "ADC A0 read %X - %d\n", adc_value, adc_value);
    fprintf(stdout, "ADC A0 read float - %.5f (Ctrl+C to exit)\n", adc_value_float);
  }
  r = mraa_aio_close(adc_a0);
  if (r != MRAA_SUCCESS) {
    mraa_result_print(r);
  }
  return r;
}
