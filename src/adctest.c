/*
 *
 * Compile with: gcc -o adctest adctest.c -lwiringPi
 *
 * If no value:
 * rmmod spi_bcm2708
 * modprobe spi_bcm2708
 *
 * http://www.icbanq.com/pbloger/board_View.aspx?number=269
 * http://www.raspberrypi.org/forums/viewtopic.php?f=93&t=78551 (baart)
 * http://www.mikroe.com/add-on-boards/measurement/adc-proto/ (boards)
 * Rpi        -   MCP3208
 * GPIO10(19) - DATA in(11)
 * GPIO9(21) - Data out (12)
 * GPIO11(23) - CLK(13)
 * GPIO8(24) - CS
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CS_MCP3208  8       // BCM_GPIO8

#define SPI_CHANNEL 0
#define SPI_SPEED   100000  // !! Start low here and if all works try to increase if needed on a breadboard I could go upto about 750000 


int read_mcp3208_adc(unsigned char adcChannel)
{
  unsigned char buff[3];
  int adcValue = 0;

  buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
  buff[1] = ((adcChannel & 0x07) << 6);
  buff[2] = 0x00;

  digitalWrite(CS_MCP3208, 0);  // Low : CS Active

  wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

  buff[1] = 0x0F & buff[1];
  adcValue = ( buff[1] << 8) | buff[2];

  digitalWrite(CS_MCP3208, 1);  // High : CS Inactive

  return adcValue;
}


int main (void)
{
  int adc1Channel = 0;
  int adc1Value   = 0;

  if(wiringPiSetup() == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    return 1 ;
  }

  if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
  {
    fprintf (stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
    return 1 ;
  }

  pinMode(CS_MCP3208, OUTPUT);

  while(1)
  {
    system("clear");
    printf("\n\nMCP3208 channel output.\n\n");
    adc1Value = read_mcp3208_adc(adc1Channel);
    printf("adc0 Value = %04u", adc1Value);
    printf("\tVoltage = %.3f\n", ((4.5/4096) * adc1Value));
    usleep(1000);
  }
  return 0;
}
