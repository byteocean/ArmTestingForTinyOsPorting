#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "stm32f4xx_conf.h"
#include "utils.h"

// Private variables
volatile uint32_t time_var1, time_var2;

volatile uint32_t flag;

// Private function prototypes
void Delay(volatile uint32_t nCount);
void init();
void calculation_test();
void dac_test();

void init_rtc();
void init_rtc_wu();

int main(void) {
	init();

	calculation_test();
//	dac_test();

	for (;;) {

	}

	return 0;
}

void dac_test() {
	for (;;) {
		for (float i = 0; i < 2.0 * M_PI; i += (2.0 * M_PI) / 100.0) {
			DAC_SetChannel1Data(DAC_Align_12b_R,
					(uint32_t) ((sinf(i) + 1.0) * 2047.0));
		}
	}
}

void calculation_test() {

	for (;;) {
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		Delay(1000);
		GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		Delay(1000);

		/*
		 time_var2 = 0;
		 for (int i = 0;i < 1000000;i++) {
		 a += 0.01 * sqrtf(a);
		 }

		 printf("Time:      %lu\n", time_var2);
		 printf("Iteration: %i\n", iteration);
		 printf("Value:     %lu\n", (unsigned long)a);
		 //		printf("Value F:   %.5f\n", -a);
		 printf("Value F2:  %s\n\n", ftostr(-a, 5));

		 iteration++;

		 */
	}
}

void init() {
	flag = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DAC_InitTypeDef DAC_InitStructure;

	// ---------- SysTick timer -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		// Capture error
		while (1) {
		};
	}

	// GPIOD Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Configure PD12, PD13, PD14 and PD15 in output pushpull mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// ------ UART ------ //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// Conf
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

	// Enable
	USART_Cmd(USART2, ENABLE);

	// ---------- DAC ---------- //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Configuration
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Enable DAC Channel1
	DAC_Cmd(DAC_Channel_1, ENABLE);

	// Set DAC Channel1 DHR12L register
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);

	//Enable RTC
	init_rtc();
	init_rtc_wu();
	GPIO_SetBits(GPIOD, GPIO_Pin_13);
//	init_rtc_wkup();
}

/*
 * Called from systick handler
 */
void timing_handler() {
	if (time_var1) {
		time_var1--;
	}

	GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	if (flag == 0) {
		//GPIO_SetBits(GPIOD, GPIO_Pin_14);
		flag = 1;
	} else {
		//GPIO_ResetBits(GPIOD, GPIO_Pin_14);
		flag = 0;
	}
	time_var2++;
}

/*
 * Delay a number of systick cycles (1ms)
 */
void Delay(volatile uint32_t nCount) {
	time_var1 = nCount;
	while (time_var1) {
	};
}

/*
 * Dummy function to avoid compiler error
 */
void _init() {

}

void init_rtc() {
	RTC_InitTypeDef RTC_InitStructure;

	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);
	/* Reset BKP Domain */
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);

	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();

	/* Configure the RTC data register and RTC prescaler */
	RTC_InitStructure.RTC_AsynchPrediv = 0x20;
	RTC_InitStructure.RTC_SynchPrediv = 0x00;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

}

void init_rtc_wu(void) {

	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Enable the RTC Wakeup Interrupt */
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	/* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	//    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
	//    RTC_SetWakeUpCounter(0x1);
	RTC_SetWakeUpCounter(0x0);

	/* Connect EXTI_Line22 to the RTC Wakeup event */
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable the RTC Wakeup Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable Wakeup Counter */
	RTC_WakeUpCmd(ENABLE);
}

