//* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
CanTxMsg TxMessage = { 0 };
CanRxMsg RxMessage = { 0 };
volatile uint8_t get_message = 0;

/* Private function prototypes -----------------------------------------------*/
void CAN_Config(void);
void USB_LP_CAN1_RX0_IRQHandler(void);

//==================================================================================================
//@brief  Main program.
int main(void) {
	// sprawdzenie taktowania zegara
	//RCC_ClocksTypeDef RCC_Clocks;
	//RCC_GetClocksFreq(&RCC_Clocks);

	/* CAN configuration */
	CAN_Config();

	/* Infinite loop */
	while (1) {

		if (get_message) {
			CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
			get_message = 0;
		}
	}
}

//==================================================================================================
/**
 * @brief  Configures the CAN.
 */
void CAN_Config(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/* NVIC configuration ***********************************/
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Clock configuration *********************************/
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/* CAN GPIOs configuration *****************************/
	/* Connect CAN pins to AF7 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_7);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_7);
	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* CAN configuration **********************************/
	CAN_StructInit(&CAN_InitStructure);
	/* CAN cell init */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	/* CAN Baudrate = 500kBps (CAN clocked at 36 MHz) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 4;

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

	CAN_Init(CAN1, &CAN_InitStructure);

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
void USB_LP_CAN1_RX0_IRQHandler(void) {

	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET) {
		get_message = 1;
	}

}
