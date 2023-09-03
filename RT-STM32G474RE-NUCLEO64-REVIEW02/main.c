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
 * [NISC2023-REVIEW02] - Using 2 ADC channels with shell.
 */

#include "ch.h"
#include "hal.h"

#include "shellconf.h"

#include "shell.h"
#include "chprintf.h"

#include <stdlib.h> /* atoi */

#define VOLTAGE_RES            ((float)3.3/4096)
#define ADC_GRP_NUM_CHANNELS   2
#define ADC_GRP_BUF_DEPTH      1

static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

int num_conversions;

/*
 * Callback called when the conversion ends
 */
static void endcallback(ADCDriver *adcp) {
  if (adcIsBufferComplete(adcp)) {
    num_conversions++;
  }
}

/*
 * ADC Linear Conversion group
 */
static const ADCConversionGroup linearcfg = {
          .circular     = false,
          .num_channels = ADC_GRP_NUM_CHANNELS,
          .end_cb       = endcallback,
          .error_cb     = NULL,
          .cfgr         = ADC_CFGR_CONT,
          .cfgr2        = 0U,
          .tr1          = ADC_TR_DISABLED,
          .tr2          = ADC_TR_DISABLED,
          .tr3          = ADC_TR_DISABLED,
          .awd2cr       = 0U,
          .awd3cr       = 0U,
          .smpr         = {
            ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_47P5) | ADC_SMPR1_SMP_AN8(ADC_SMPR_SMP_47P5),
            0U
          },
          .sqr          = {
            ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN8),
            0U,
            0U,
            0U
          }
        };

/* Converts values in linear mode */
static void cmd_joystick(BaseSequentialStream *chp, int argc, char *argv[]) {
  if(argc > 1){
    chprintf( chp, "Usage: joystick [num_seconds]\n\r" );
    return;
  }

  num_conversions = 0;

  chprintf( chp, "Starting conversion in linear mode...\n\r" );

  if( argc == 1 ){
    unsigned int seconds = atoi(argv[0]);
    /* The cycle waits for the user to press a key */
    while ( chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT ) {
      /* One-shot conversion */
      adcConvert(&ADCD1, &linearcfg, samples, ADC_GRP_BUF_DEPTH);
      chprintf(chp, "Value 1: %f\n\r", (float) samples[0] * VOLTAGE_RES);
      chprintf(chp, "Value 2: %f\n\r", (float) samples[1] * VOLTAGE_RES);
      chprintf(chp, "Number of conversions: %d\n\r\n\r", num_conversions);
      chThdSleepSeconds( seconds );
    }
  }
  else{
    /* One-shot conversion */
    adcConvert(&ADCD1, &linearcfg, samples, ADC_GRP_BUF_DEPTH);
    chprintf(chp, "Value 1: %f\n\r", (float) samples[0] * VOLTAGE_RES);
    chprintf(chp, "Value 2: %f\n\r\n\r", (float) samples[1] * VOLTAGE_RES);
  }
}


static const ShellCommand commands[] = {
  {"joystick", cmd_joystick},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
};

#define WA_SHELL   2048
THD_WORKING_AREA( waShell, WA_SHELL );

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  /*
   * Initializes a SERIAL driver.
   */
  palSetPadMode( GPIOA, 2, PAL_MODE_ALTERNATE(7) );
  palSetPadMode( GPIOA, 3, PAL_MODE_ALTERNATE(7) );

  /*
   * Correspondence with ADC1_IN7 and ADC1_IN8
   */
  palSetPadMode( GPIOC, 1, PAL_MODE_INPUT_ANALOG );
  palSetPadMode( GPIOC, 2, PAL_MODE_INPUT_ANALOG );

  sdStart(&SD2, NULL);

  adcStart(&ADCD1, NULL);
  adcSTM32EnableVREF(&ADCD1);

  shellInit();

  /*
   * Create Shell Thread!
   */
  chThdCreateStatic(waShell, sizeof(waShell), NORMALPRIO, shellThread, (void *)&shell_cfg1);

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
