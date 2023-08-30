/*
    NeaPolis Innovation Summer Campus 2023 Examples
    Copyright (C) 2020-2023 Salvatore Dello Iacono [delloiaconos@gmail.com]

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * [NISC2023-SERIAL02] - ChibiOS/HAL SERIAL Driver Example 02
 * DESCRIPTION: How to use Streams.
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chscanf.h"

BaseSequentialStream * chp = (BaseSequentialStream *) &SD2;

#define WA_SERIAL_SIZE      1024
THD_WORKING_AREA( waSerial, WA_SERIAL_SIZE );
THD_FUNCTION( thdSerial, arg ) {
  (void) arg;

  chRegSetThreadName( "serial" );

  /* Port Configuration: PA2 and PA3 connected to Debugger Emulated Serial Port .
   * Serial Driver 2 works on STM32 USART2 Peripheral!
   */
  palSetPadMode( GPIOA, 2, PAL_MODE_ALTERNATE(7) );
  palSetPadMode( GPIOA, 3, PAL_MODE_ALTERNATE(7) );

  /* Starting Serial Driver #2 */
  sdStart( &SD2, NULL );

  while( true ) {
    int start, stop, count;
    chprintf( chp, "Start:\r\n" );
    chscanf( (BaseBufferedStream*) chp, "%d", &start );
    chprintf( chp, "%d\r\n", start );
    chprintf( chp, "Stop:\r\n" );
    chscanf( (BaseBufferedStream*) chp, "%d", &stop );
    chprintf( chp, "%d\r\n", stop );

    if( stop > start) {
      for( count = start; count <= stop; count++ ) {
        chprintf( chp, "%d ", count );
      }
      chprintf( chp, "\r\n\n" );
    } else {
      chprintf( chp, "Error!\r\n" );
    }
    chThdSleepMilliseconds( 100 );
  }
}


int main(void) {

  halInit();
  chSysInit();

  chThdCreateStatic( waSerial, sizeof(waSerial), NORMALPRIO-1, thdSerial, NULL);

  while( true ) {
    palToggleLine( LINE_LED_GREEN );
    chThdSleepMilliseconds( 500 );
  }
}
