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
 * [PWM01] Using PWM Peripheral - Example 01
 * Simple project to calibrate the SG90 Servo.
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

BaseSequentialStream *chp = (BaseSequentialStream*) &SD2;

/* From Servo SG90 datasheet (can be found online)
 * PWM Period is 20 ms
 * 0   is middle                      (1.5ms pulse)         DT = 7.50%
 * 90  is all the way to the right    (about 2ms pulse)     DT = 10.00%
 * -90 is all the way to the left     (about 1ms pulse)     DT = 5.00%
*/

/* Actual values after testing:
 * 0   is middle                      (1.5ms pulse)     DT = 7.50%
 * 90  is all the way to the right    (2.5ms pulse)     DT = 12.50%
 * -90 is all the way to the left     (0.5ms pulse)     DT = 2.50%
*/

#define PWM_TIMER_FREQUENCY     1000000                               /*  Ticks per second  */
#define PWM_PERIOD              (PWM_TIMER_FREQUENCY * 20 / 1000)     /*  The SERVO SG90 runs on a 20ms period  */

void pwmWidtchCb(PWMDriver *pwmp){
  (void)pwmp;
}

/*
 * Configures PWM Driver.
 */
static PWMConfig pwmcfg = {
  .frequency = PWM_TIMER_FREQUENCY,     /*  Timer clock in Hz   */
  .period = PWM_PERIOD,                 /*  PWM Period in ticks */
  .callback = pwmWidtchCb,              /*  Callback            */
  .channels = {                         /*  PWM3 has 4 channels */
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  }
};

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
   * Pin for PWM output. B4 = D5
   */
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2));

  /*
   * Pins to initialize Serial driver.
   */
  palSetPadMode( GPIOA, 2, PAL_MODE_ALTERNATE(7) );
  palSetPadMode( GPIOA, 3, PAL_MODE_ALTERNATE(7) );
  sdStart(&SD2, NULL);

  chprintf(chp, "Number of ticks in one period: %d\n\r", PWM_PERIOD);
  pwmStart(&PWMD3, &pwmcfg);

  int i= 250;

  /*
   * PWM_PERCENTAGE_TO_WIDTH takes as parameters the PWM driver and a percentage (250 -> 2.50%)
   */
  pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, i));

  while (true) {
    if(palReadLine( LINE_BUTTON ) == PAL_HIGH){
      pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, i));
      chprintf(chp, "Percentage: %d\n\r", i);
      switch(i){
        case 250:
          chprintf(chp, "The SERVO should be 90 degrees to the left.\n\r");
          break;
        case 750:
          chprintf(chp, "The SERVO should be in the middle.\n\r");
          break;
        case 1250:
          chprintf(chp, "The SERVO should be 90 degrees to the right.\n\r");
          break;
        default:
          break;
      }
      i+=50;
      while(palReadLine( LINE_BUTTON ) == PAL_HIGH){}
    }
  }

  pwmDisableChannel(&PWMD3, 0);

  pwmStop(&PWMD3);
}
