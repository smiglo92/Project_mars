/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_StdPeriph_Examples
 *  @addtogroup CAN_Networking
 */

/* Private variables ---------------------------------------------------------*/
CAN_InitTypeDef CAN_InitStructure;
CAN_FilterInitTypeDef CAN_FilterInitStructure;
CanTxMsg TxMessage;
uint8_t ubKeyNumber = 0x0;

/* Private function prototypes -----------------------------------------------*/
static void NVIC_Config(void);
static void CAN_Config(void);
void Init_RxMes(CanRxMsg *RxMessage);
void GPIO_init();
void delay(int time);

//===================================================================================================
int main(void) {
// sprawdzenie taktowania zegara
//	RCC_ClocksTypeDef RCC_Clocks;
//	RCC_GetClocksFreq(&RCC_Clocks);

	/* NVIC configuration */
	NVIC_Config();

	/* GPIO configuration */
	GPIO_init();

	/* CAN configuration */
	CAN_Config();

	while (1) {

		TxMessage.Data[0] = 2;
		CAN_Transmit(CAN1, &TxMessage);
		/* Wait until one of the mailboxes is empty */
		while ((CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP0) != RESET)
				|| (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP1) != RESET)
				|| (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP2) != RESET))
			;

	}
}

//==================================================================================================
/**
 * @brief  Configures the NVIC for CAN.
 * przerwanie od rejestru odbiorczego RX0 dla CAN1
 */
static void NVIC_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//==================================================================================================
/** Inicjalizacaj GPIO PC0 PC1 jako wyjścia
 * PC0 - czerwona dioda
 * PC1 - zolta dioda
 */
void GPIO_init() {
	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_StructInit(&gpio); // domyslna konfiguracja
	gpio.GPIO_Pin = GPIO_Pin_0; // konfigurujemy pin 5
	gpio.GPIO_Mode = GPIO_Mode_OUT;  //ko wyjscie
	GPIO_Init(GPIOC, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_1; // konfigurujemy pin 5
	gpio.GPIO_Mode = GPIO_Mode_OUT;  //ko wyjscie
	GPIO_Init(GPIOC, &gpio);
}
//==================================================================================================
/**
 * @brief  Configures the CAN.
 * @param  None
 * @retval None
 */
static void CAN_Config(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t sukces = 0;

	/* CLOCK enable */

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/*GPIO INIT*/
	/* Connect CAN pins to AF9 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	/*GPIO CAN1 init*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* CAN configuration set parameters */
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	/* CAN Baudrate = 1 MBps (CAN clocked at 30 MHz) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 3;
	/* CAN filter init */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

	/* CAN register init */
	CAN_DeInit(CAN1);
	sukces = CAN_Init(CAN1, &CAN_InitStructure);
	/* sukces = 0   blad inicjalizacji
	 * sukces = 1  inicjalizacja zakończona pomyslnie
	 */
	//zapalenie diod zaleznie od wyniku inicjalizacji
	if (sukces == 1) {
		GPIO_SetBits(GPIOC, GPIO_Pin_1);
		GPIO_SetBits(GPIOC, GPIO_Pin_0);
	}
	if (sukces == 0) {
		GPIO_ResetBits(GPIOC, GPIO_Pin_0);
	}
	//inicjalizacja filtów CAN1
	CAN_FilterInit(&CAN_FilterInitStructure);

	/* Transmit Structure preparation */
	TxMessage.StdId = 0x321;
	TxMessage.ExtId = 0x01;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = CAN_ID_STD;
	TxMessage.DLC = 1;
	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

}
//==================================================================================================
/**
 * @brief  Initializes the Rx Message.
 * @param  RxMessage: pointer to the message to initialize
 * @retval None
 */
void Init_RxMes(CanRxMsg *RxMessage) {
	uint8_t ubCounter = 0;

	RxMessage->StdId = 0x00;
	RxMessage->ExtId = 0x00;
	RxMessage->IDE = CAN_ID_STD;
	RxMessage->DLC = 0;
	RxMessage->FMI = 0;
	for (ubCounter = 0; ubCounter < 8; ubCounter++) {
		RxMessage->Data[ubCounter] = 0x00;
	}
}

//==================================================================================================
void delay(int time) {
	int i;
	for (i = 0; i < time * 4000; i++) {
	}
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
