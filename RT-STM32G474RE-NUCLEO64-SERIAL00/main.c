/*
    NeaPolis Innovation Summer Campus Examples
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
 * [NISC2023-SERIAL00] - ChibiOS/HAL SERIAL Driver Example 00.
 * DESCRIPTION: Simple Echoback example!
 */

#include "ch.h"
#include "hal.h"

#define WA_SERIAL_SIZE 256
THD_WORKING_AREA( waSerial, WA_SERIAL_SIZE);
THD_FUNCTION( thdSerial, arg ) {
  (void) arg;
  chRegSetThreadName( "serial" );

  /*
   * Port configuration
   */
  palSetPadMode( GPIOA, 2, PAL_MODE_ALTERNATE(7) );
  palSetPadMode( GPIOA, 3, PAL_MODE_ALTERNATE(7) );

  /* Starting Serial Driver #2 */
  sdStart( &SD2, NULL );

  while (true) {
    char c = sdGet( &SD2 );
    sdPut( &SD2, c );
  }
}

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  chThdCreateStatic( waSerial, sizeof(waSerial), NORMALPRIO-1, thdSerial, NULL);

  while (true) {
    palToggleLine( LINE_LED_GREEN );
    chThdSleepMilliseconds(500);
  }
}
