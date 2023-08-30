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
#include "notes.h"

#define PORTAB_LINE_LED1            LINE_LED
#define PORTAB_LED_OFF              PAL_LOW
#define PORTAB_LED_ON               PAL_HIGH
#define PORTAB_LINE_BUTTON          LINE_BUTTON
#define PORTAB_BUTTON_PRESSED       PAL_HIGH

#define PORTAB_SD1                  LPSD1

#define PORTAB_DAC_TRIG             7


/*
 * DAC streaming callback.
 */
size_t nx = 0, ny = 0, nz = 0;
static void end_cb1(DACDriver *dacp) {

  nz++;
  if (dacIsBufferComplete(dacp)) {
    nx += DAC_BUFFER_SIZE / 4;
  }
  else {
    ny += DAC_BUFFER_SIZE / 4;
  }

  if ((nz % 1000) == 0) {
    palToggleLine(PORTAB_LINE_LED1);
  }
}

/*
 * DAC error callback.
 */
static void error_cb1(DACDriver *dacp, dacerror_t err) {

  (void)dacp;
  (void)err;

  chSysHalt("DAC failure");
}

static const DACConfig dac1cfg1 = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

static const DACConversionGroup dacgrpcfg1 = {
  .num_channels = 1U,
  .end_cb       = end_cb1,
  .error_cb     = error_cb1,
  .trigger      = DAC_TRG(PORTAB_DAC_TRIG)
};

/*
 * GPT6 configuration.
 */
static const GPTConfig gpt6cfg1 = {
  .frequency    = 2*DAC_BUFFER_SIZE*NOTE_G5,
  .callback     = NULL,
  .cr2          = TIM_CR2_MMS_1,    /* MMS = 010 = TRGO on Update Event.    */
  .dier         = 0U
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

  /* Setting up the output pin as analog as suggested
       by the Reference Manual.*/
  palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);

  /* Starting DAC1 driver.*/
  dacStart(&DACD1, &dac1cfg1);

  /* Starting GPT6 driver, it is used for triggering the DAC.*/
  gptStart(&GPTD6, &gpt6cfg1);


  /*
   * Normal main() thread activity, if the button is pressed then the DAC
   * transfer is stopped.
   */
  while (true) {
	  /* Starting a continuous conversion.*/
	    dacStartConversion(&DACD1, &dacgrpcfg1,
	                       (dacsample_t *)dac_buffer, DAC_BUFFER_SIZE);
	    gptStartContinuous(&GPTD6, 2U);

    if (palReadLine(PORTAB_LINE_BUTTON) == PORTAB_BUTTON_PRESSED) {
      gptStopTimer(&GPTD6);
      dacStopConversion(&DACD1);
    }
    chThdSleepMilliseconds(500);
  }
  return 0;
}
