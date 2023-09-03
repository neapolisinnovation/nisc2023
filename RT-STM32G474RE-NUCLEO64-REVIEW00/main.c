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
 * [NISC2023-REVIEW00]
 */

#include "ch.h"
#include "hal.h"

#define RLED_LINE   PAL_LINE( GPIOB, 5U )
#define GLED_LINE   PAL_LINE( GPIOA, 10U )
#define BLED_LINE   PAL_LINE( GPIOB, 3U )

#define BTN_PORT GPIOA
#define BTN_PAD 8U

#define BLINKING_HALF_PERIOD_MILLISECONDS 200

const ioline_t rgb_lines[] = {RLED_LINE,GLED_LINE,BLED_LINE};
#define RGB_LINES_SIZE 3

typedef struct {
  uint8_t selected_led;
  mutex_t mutex;
} shared_t;

/*
 * RGB LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waRGBBlinker, 1024);
static THD_FUNCTION(thdRGBBlinker, arg) {

  shared_t* shared = (shared_t*)arg;
  for(uint8_t i=0;i<RGB_LINES_SIZE;i++) {
        palSetLineMode(rgb_lines[i],PAL_MODE_OUTPUT_PUSHPULL);
        palClearLine(rgb_lines[i]);
   }


  while(TRUE) {
    chMtxLock(&shared->mutex);
    palSetLine(rgb_lines[shared->selected_led]);
    chThdSleepMilliseconds(BLINKING_HALF_PERIOD_MILLISECONDS);
    palClearLine(rgb_lines[shared->selected_led]);
    chMtxUnlock(&shared->mutex);
    chThdSleepMilliseconds(BLINKING_HALF_PERIOD_MILLISECONDS);
  }
}

/*
 * Thread that detects button press
 */
static THD_WORKING_AREA(waButton, 1024);
static THD_FUNCTION(thdButton, arg) {
  shared_t* shared = (shared_t*)arg;
  palSetPadMode(BTN_PORT,BTN_PAD,PAL_MODE_INPUT_PULLUP);
  while(TRUE) {
    if(palReadPad(BTN_PORT,BTN_PAD)==PAL_LOW) {
        chThdSleepMilliseconds(5);
        while(palReadPad(BTN_PORT,BTN_PAD)==PAL_LOW) {
          chThdSleepMilliseconds(10);
        }
        chMtxLock(&shared->mutex);
        shared->selected_led = (shared->selected_led+1)%RGB_LINES_SIZE;
        chMtxUnlock(&shared->mutex);
      }
     chThdSleepMilliseconds(10);
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


  shared_t shared;

  chMtxObjectInit( &shared.mutex );
  shared.selected_led = 0;

  chThdCreateStatic( waButton, sizeof( waButton ), NORMALPRIO-1, thdButton, (void*) &shared );
  chThdCreateStatic( waRGBBlinker, sizeof( waRGBBlinker ), NORMALPRIO+1, thdRGBBlinker, (void*) &shared );

  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
