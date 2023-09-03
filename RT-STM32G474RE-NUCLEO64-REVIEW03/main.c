/*
    NeaPolis Innovation Summer Campus 2022 Examples
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
 * [NISC2023-SHELL01] - Static shell thread with custom commands and configuration.
 * DESCRIPTION: Demo modified to accept new commands, removed EXIT and TEST commands
 * from standard configuration.
 * - Tests commands are disabled
 * - Exit command is disabled
 */

#include "ch.h"
#include "hal.h"

#include "shellconf.h"

#include "shell.h"
#include "chprintf.h"

#include <stdlib.h> /* atoi */

#define VOLTAGE_RES            ((float)3.3/4096)
#define ADC_GRP_NUM_CHANNELS   1
#define ADC_GRP_BUF_DEPTH      16

static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

int num_conversions;

/*
 * GPT4 configuration. This timer is used as trigger for the ADC.
 */
const GPTConfig gpt4cfg = {
  .frequency    =  1000000U,
  .callback     =  NULL,
};

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
          .end_cb       = NULL,
          .error_cb     = NULL,
          .cfgr         = ADC_CFGR_CONT,
          .cfgr2        = 0U,
          .tr1          = ADC_TR_DISABLED,
          .tr2          = ADC_TR_DISABLED,
          .tr3          = ADC_TR_DISABLED,
          .awd2cr       = 0U,
          .awd3cr       = 0U,
          .smpr         = {
            ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_247P5),
            0U
          },
          .sqr          = {
            ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7),
            0U,
            0U,
            0U
          }
        };
/*
 * ADC Circular Conversion group
 */
const ADCConversionGroup circularcfg = {
          .circular     = true,
          .num_channels = ADC_GRP_NUM_CHANNELS,
          .end_cb       = endcallback,
          .error_cb     = NULL,
          .cfgr         = ADC_CFGR_CONT |
                          ADC_CFGR_EXTEN_RISING |
                          ADC_CFGR_EXTSEL_SRC(12),  /* TIM4_TRGO */
          .cfgr2        = 0U,
          .tr1          = ADC_TR_DISABLED,
          .tr2          = ADC_TR_DISABLED,
          .tr3          = ADC_TR_DISABLED,
          .awd2cr       = 0U,
          .awd3cr       = 0U,
          .smpr         = {
            ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_247P5),
            0U
          },
          .sqr          = {
            ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7),
            0U,
            0U,
            0U
          }
        };


/* Returns the average of the converted values */
static float find_avg(void){
  float avg = 0.0f;
  for( int i = 0; i < ADC_GRP_BUF_DEPTH; i++ ) {
    avg += (float) samples[i] * VOLTAGE_RES;
  }
  avg /= ADC_GRP_BUF_DEPTH;
  return avg;
}


/* Converts values in linear mode */
static void cmd_linear(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  chprintf(chp, "Starting conversion in linear mode...\n\r", find_avg());
  adcConvert(&ADCD1, &linearcfg, samples, ADC_GRP_BUF_DEPTH);
  chprintf(chp, "Average found: %f\n\r", find_avg());
}


/* Converts values in circular mode */
static void cmd_circular(BaseSequentialStream *chp, int argc, char *argv[]) {

  if( argc != 1 ) {
    chprintf(chp, "Usage: circular num_seconds\n\r");
    return;
  }

  num_conversions = 0;

  chprintf(chp, "Starting conversion in circular mode...\n\r", find_avg());

  adcStartConversion(&ADCD1, &circularcfg, samples, ADC_GRP_BUF_DEPTH);

  /*
   * Start the GPT4 driver with a period of 10000 cycles and a
   * frequency of 1000000 Hz
   */
  gptStartContinuous(&GPTD4, 10000);

  while ( chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT ) {
    chThdSleepSeconds( atoi(argv[0]) );
    chprintf(chp, "Average found: %f\n\r", find_avg());
    chprintf(chp, "Number of conversions: %d\n\r\n\r", num_conversions);
  }

  adcStopConversion( &ADCD1 );
}


static const ShellCommand commands[] = {
  {"linear", cmd_linear},
  {"circular", cmd_circular},
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
   * Correspondence with ADC12_IN7
   */
  palSetPadMode( GPIOC, 1, PAL_MODE_INPUT_ANALOG );

  sdStart(&SD2, NULL);

  /*
   * Starting GPT4 driver, it is used for triggering the ADC.
   * Starting the ADC1 driver.
   */
  gptStart(&GPTD4, &gpt4cfg);

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
