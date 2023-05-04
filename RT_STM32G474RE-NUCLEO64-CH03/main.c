/*
    NeaPolis Innovation Summer Campus 2022 Examples
    Copyright (C) 2020-2022 Salvatore Attanasio [salva0729@gmail.com]

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
 * [NISC2022-PWM04] - PWM Example 04
 * DESCRIPTION: Usage of PWM for control of SG90 Servo.
 * Target position is given by an analog input from a potenziometer
 * sampled by ADC.
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"


/*
 * ADC Settings
 */
#define ADC1_CH_NUM 1       // number of used adc channels
#define POT_SAMPLES_NUM 10  // number of samples
static adcsample_t sample_buff[POT_SAMPLES_NUM];    // adc buffer

/* From Servo SG90 datasheet (can be found online)
 * PWM Period is 20 ms
 * 0   is middle                      (1.5ms pulse)         DT = 7.50%
 * 90  is all the way to the left     (about 2ms pulse)     DT = 10.00%
 * -90 is all the way to the right    (about 1ms pulse)     DT = 5.00%
*/

/* Actual values after testing:
 * 0   is middle                      (1.5ms pulse)     DT = 7.50%
 * 90  is all the way to the left     (2.3ms pulse)     DT = 12.00%
 * -90 is all the way to the right    (0.5ms pulse)     DT = 2.50%
*/

#define MIDDLE_POSITION_DT_PP   750U
#define MAX_POSITION_DT_PP      1200U
#define MIN_POSITION_DT_PP      250U

static bool DATA_READY = FALSE;
static int32_t POT_VALUE = 0;

/*
 * ADC Configuration.
 */
/*
static const ADCConversionGroup pot_adccg = {
  FALSE,
  (uint16_t)(ADC1_CH_NUM),
  NULL,                 // end callback
  NULL,                 // error callback
  0,                    //CR1
  ADC_CR2_SWSTART,      //CR2
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_3),
  0,
  0,
  0,
  0,
  0,
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};
*/
static const ADCConversionGroup adcgrpcfg1 = {
  .circular     = false,
  .num_channels = ADC1_CH_NUM,
  .end_cb       = NULL,
  .error_cb     = NULL,
  .cfgr         = 0U,
  .cfgr2        = 0U,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_247P5),
    0U
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    0U,
    0U,
    0U
  }
};



/*
 * PWM Driver Configuration.
 */
static const PWMConfig pwmcfg = {
  10000,                            // 10kHz PWM clock fequency
  200,                              // PWM period is 20ms
  NULL,                             // Period callback
  {                                 // Channels mode and callback
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},  // CH1
   {PWM_OUTPUT_DISABLED, NULL},     // CH2
   {PWM_OUTPUT_DISABLED, NULL},     // CH3
   {PWM_OUTPUT_DISABLED, NULL}      // CH4
  },
  0,
  0
};

/*
 * Potenziometer Read Thread
 * ADC is used to read signal from Potenziometer
 * multiple samples are recorded to make some noise reduction
 */
static THD_WORKING_AREA(waPotenziometerReadThd, 512);
static THD_FUNCTION(PotenziometerReadThd, arg){
  (void) arg;
  chRegSetThreadName("Potenziometer Sampler");

  // Set PA0 as Analog input
  palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG);

  // Start ADC Driver
  adcStart(&ADCD1, NULL);

  while(TRUE){
    adcConvert(&ADCD1, &adcgrpcfg1, (adcsample_t*) sample_buff, POT_SAMPLES_NUM);

    // computing mean value
    int value = 0;
    for(int i=0; i<POT_SAMPLES_NUM; i++){
      value += sample_buff[i];
    }

    value /= POT_SAMPLES_NUM;

    POT_VALUE = value;
    DATA_READY = TRUE;
    chThdSleepMilliseconds(20);
  }
}

/*
 * Serial Print Thread
 */
static THD_WORKING_AREA(waSerialPrintThd, 128);
static THD_FUNCTION(SerialPrintThd, arg){
  (void) arg;
  chRegSetThreadName("PrintToSD");

  sdStart(&SD2, NULL);
  while(TRUE){
    // remember to set CHPRINTF_USE_FLOAT TRUE in chprintf.h
    chprintf((BaseSequentialStream*)&SD2,"Valore potenziomentro %.2f% !\r\n", POT_VALUE / 4096.0  * 100);
  }
}

/*
 * Servo Control Thread
 * Servo goes to position based on signal read in PotenziometerReadThd
 */
static THD_WORKING_AREA(waServoThd, 128);
static THD_FUNCTION(ServoThd, arg) {
  (void)arg;
  chRegSetThreadName("Servo Control");

  // Set PB4 for PWM use
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2));

  // Start Driver
  pwmStart(&PWMD3, &pwmcfg); // pwm on timer 3

  int target = 0;            // target angular postition (percentage)

  while(TRUE){
    if(DATA_READY){
      // POT_VALUE is a value between 0 and 4096
      // target is a percentage value between MIN_POSITION_DT_PP and MAX_POSITION_DT_PP
      target = (POT_VALUE/4096.0) * (MAX_POSITION_DT_PP - MIN_POSITION_DT_PP) + MIN_POSITION_DT_PP;
      pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, target));
      DATA_READY = FALSE;
    }
    chThdSleepMilliseconds(3);
  }
}

int main(void) {
  halInit();
  chSysInit();


  // Create Threads
  chThdCreateStatic(waPotenziometerReadThd, sizeof(waPotenziometerReadThd), NORMALPRIO + 1, PotenziometerReadThd, NULL);
  chThdCreateStatic(waSerialPrintThd, sizeof(waSerialPrintThd), NORMALPRIO + 1, SerialPrintThd, NULL);
  chThdCreateStatic(waServoThd, sizeof(waServoThd), NORMALPRIO + 1, ServoThd, NULL);

  while (true) {
    // do nothing
    chThdSleepMilliseconds(200);
  }
}
