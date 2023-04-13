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
 * [NISC2023-GPIO08] - How to use PAL Events and callbacks to measure time.
 * DESCRIPTION: Use of event callbacks to measure time with Virtual Timer System Time.
 * NOTE: Enable PAL_USE_CALLBACKS in halconf.h
 */

#include "ch.h"
#include "hal.h"
/**
 * include chprintf in order to print to serial.
 */
//#include "chprintf.h"

/*
 * Shared Variable: time
 */
static uint32_t time;

static void button_cb(void *arg) {

  (void)arg;

  static systime_t start, stop;
  chSysLockFromISR();
  if( palReadLine( LINE_BUTTON ) == PAL_LOW ) {
    start = chVTGetSystemTimeX();
    /**
     * the following line prints the timestamp of the callback
     */
    //chprintf((BaseSequentialStream*)&LPSIOD1,"%ul\r\n",start);
  } else {
    sysinterval_t delta;
    stop = chVTGetSystemTimeX();
    delta = chTimeDiffX( start, stop );
    time = TIME_I2MS( delta );
    /**
     * the following line prints the duration of the callback
     */
    //chprintf((BaseSequentialStream*)&LPSIOD1,"%ul\r\n",time);
  }
  chSysUnlockFromISR();
}

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  /**
   * this lines are needed in order to print to serial
   */
  //sioStart(&LPSIOD1, NULL);
  //sioStartOperation(&LPSIOD1, NULL);

  /* Enabling events on both edges of the button line.*/
  palEnableLineEvent( LINE_BUTTON, PAL_EVENT_MODE_BOTH_EDGES);
  palSetLineCallback( LINE_BUTTON, button_cb, NULL);

  while (true) {
    chThdSleepMilliseconds( 1000 );
  }
}
