/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

BaseSequentialStream * chp = (BaseSequentialStream *) &SD2;



/* True Button: Port, Pin, Line */
#define TBTN_PORT     GPIOC
#define TBTN_PIN      7U
#define TBTN_LINE     PAL_LINE( TBTN_PORT, TBTN_PIN )

/* False Button: Port, Pin, Line */
#define FBTN_PORT     GPIOA
#define FBTN_PIN      4U
#define FBTN_LINE     PAL_LINE( FBTN_PORT, FBTN_PIN )


#define NUM_DOM		2
#define TIME_LIMIT	5


uint8_t ans = 0;
uint8_t playing = 0;

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("ques_check");
  while (true) {
	  if (playing){
		  if (ans == 1 && palReadLine(TBTN_LINE) || ans == 0 && palReadLine(FBTN_LINE)){
		  chSysLock();
		  playing = 0;
		  chSysUnlock();
		  }
	  }
  }
}


void timer(){
	for(uint8_t i = TIME_LIMIT; i >= 1 && playing == 1; i--){
		chprintf(chp, "%d... \n\r", &i);
	    chThdSleepMilliseconds(1000);

	}

}


uint8_t domande( uint8_t index ){
	switch(index){
	case 0:
		chprintf(chp, "Salvatore Attanasio è scemo? V/F \n\r");
		return 1;
	case 1:
		chprintf(chp, "La F4 è migliore della G4? V/F \n\r");
		return 0;
	default:
		chprintf(chp, "Error \n\r");
		return 0;
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



  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
	sdStart(&SD2, NULL);

  /*
   * Creates the blinker thread.
   */
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */

	uint8_t current_ques = 1;

	while (true) {
		if(current_ques != NUM_DOM && playing == 0){
		    chSysLock();
			ans = domande(current_ques);
			playing = 1;
			timer();
			chSysUnlock();
			current_ques++;
		}

   if (palReadLine(LINE_BUTTON)) {
    }
    chThdSleepMilliseconds(500);
 }
}

