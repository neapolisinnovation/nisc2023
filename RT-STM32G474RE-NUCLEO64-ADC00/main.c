/*
    NeaPolis Innovation Summer Campus 2023 Examples

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
 * [ADC00] Using ADC Peripherals - Example 00
 * Base demo to include the ADC into your NUCLEO64 based project.
 */

#include "ch.h"
#include "hal.h"

#define ADC_GRP1_NUM_CHANNELS       2
#define ADC_GRP1_BUF_DEPTH          1

#define ADC_GRP2_NUM_CHANNELS        2
#define ADC_GRP2_BUF_DEPTH           64

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t samples2[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
size_t n = 0, nx = 0, ny = 0;
static void adccallback(ADCDriver *adcp) {

  n++;
  if (adcIsBufferComplete(adcp)) {
    nx += 1;
  }
  else {
    ny += 1;
  }
  if ((n % 200) == 0U) {
	  palToggleLine(LINE_LED_GREEN);
  }
}

/*
 * ADC error callback.
 */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 * Mode:        One shot, 2 channels, SW triggered.
 * Channels:    IN1, IN2.
 */
static const ADCConversionGroup adcgrpcfg1 = {
		  .circular     = false,
		  .num_channels = ADC_GRP1_NUM_CHANNELS,
		  .end_cb       = NULL,
		  .error_cb     = adcerrorcallback,
		  .cfgr         = 0U,
		  .cfgr2        = 0U,
		  .tr1          = ADC_TR_DISABLED,
		  .tr2          = ADC_TR_DISABLED,
		  .tr3          = ADC_TR_DISABLED,
		  .awd2cr       = 0U,
		  .awd3cr       = 0U,
		  .smpr         = {
		    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_247P5) |
		    ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_247P5),
		    0U
		  },
		  .sqr          = {
		    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
		    0U,
		    0U,
		    0U
		  }
		};


/*
 * ADC conversion group.
 * Mode:        Continuous, 2 channels, HW triggered by GPT4-TRGO.
 * Channels:    IN1, IN2.
 */
static const ADCConversionGroup adcgrpcfg2 = {
		  .circular     = true,
		  .num_channels = ADC_GRP2_NUM_CHANNELS,
		  .end_cb       = adccallback,
		  .error_cb     = adcerrorcallback,
		  .cfgr         = ADC_CFGR_EXTEN_RISING |
		                  ADC_CFGR_EXTSEL_SRC(12),  /* TIM4_TRGO */
		  .cfgr2        = 0U,
		  .tr1          = ADC_TR_DISABLED,
		  .tr2          = ADC_TR_DISABLED,
		  .tr3          = ADC_TR_DISABLED,
		  .awd2cr       = 0U,
		  .awd3cr       = 0U,
		  .smpr         = {
		    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_247P5) |
		    ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_247P5),
		    0U
		  },
		  .sqr          = {
		    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
		    0U,
		    0U,
		    0U
		  }
		};


/*
 * GPT configuration.
 */
const GPTConfig gptcfg1 = {
  .frequency    =  1000000U,
  .callback     =  NULL,
  .cr2          =  TIM_CR2_MMS_1,   /* MMS = 010 = TRGO on Update Event.    */
  .dier         =  0U
};

/*
 * Application entry point.
 */
int main(void) {

  halInit();
  chSysInit();

  /*
   * Setting up analog inputs used by the demo.
   */
  palSetGroupMode(GPIOC, PAL_PORT_BIT(1) | PAL_PORT_BIT(2),
                  0, PAL_MODE_INPUT_ANALOG);

  /*
   * Activates the ADC1 driver and the temperature sensor.
   */
  adcStart(&ADCD1, NULL);
  adcSTM32EnableVREF(&ADCD1);
  adcSTM32EnableTS(&ADCD1);


 /*
  * Performing a one-shot conversion on two channels.
  */
  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  cacheBufferInvalidate(samples1, sizeof (samples1) / sizeof (adcsample_t));


 /*
  * Starting PORTAB_GPT1 driver, it is used for triggering the ADC.
  */
  gptStart(&GPTD1, &gptcfg1);

  /*
   * Starting an ADC continuous conversion triggered with a period of
   * 1/10000 second.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg2,samples2, ADC_GRP2_BUF_DEPTH);
  gptStartContinuous(&GPTD1, 100U);


  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg2, samples2, ADC_GRP2_BUF_DEPTH);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    if (palReadLine( LINE_BUTTON ) == PAL_LOW) {
      adcStopConversion(&ADCD1);
      adcSTM32DisableVREF(&ADCD1);
    }
    chThdSleepMilliseconds(500);
  }
}
