/*
 	Neapolis Innovation - Copyright (C) 2023 Salvatore Bramante
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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

BaseSequentialStream *chp = (BaseSequentialStream *)&SD2;


/*
 * TRNG thread that prints a random number each 100ms
 */
static THD_WORKING_AREA(waThread1, 1024);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("trng");
  size_t size = 2;
  uint8_t out;

  //trng driver start
  trngStart(&TRNGD1, NULL);

  while (true) {
	  //trng number generation
	  trngGenerate(&TRNGD1, size, &out);
	  chprintf( chp, "\033[H" );
	  chprintf( chp, "rand number: %d\n\r", out );
	  chThdSleepMilliseconds(100);

  }
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();


  //Per driver seriale
   palSetPadMode(GPIOA, 2U, PAL_MODE_ALTERNATE(7));
   palSetPadMode(GPIOA, 3U, PAL_MODE_ALTERNATE(7));
  /*
   * Activates the Serial or SIO driver using the default configuration.
   */
  sdStart(&SD2, NULL);
  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, NULL);


  while (true) {
    chThdSleepMilliseconds(500);
  }
}

