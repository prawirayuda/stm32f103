/*
 * serial_communication.c
 *
 *  Created on: Jun 20, 2023
 *      Author: muham
 */


#include "serial_communication.h"
#include "atc_processor.h"
//#include "socket.h"
#include "string.h"
#include "stm32f1xx.h"

// TODO: May need to make baudrate configurable
#define INTERNAL_UART_BAUDRATE 115200
#define MAX_UART_DATA_QUEUE 4
#define UART_DMA_TX_TIMEOUT_MAX UART_BUFFER_SIZE * 1000 / 115200



static void intCommTxCompleted_cb(void *);
static void intCommRxIncoming_cb(void *);

static void extCommTxCompleted_cb(void *);
static void extCommRxIncoming_cb(void *);

//static void UART_EndRxTransfer(UART_HandleTypeDef *huart);
//static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma);
//static void UART_EndTransmit_IT(UART_HandleTypeDef *huart);


static HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart);
static void UART_EndRxTransfer(UART_HandleTypeDef *huart);
static HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart);
static HAL_StatusTypeDef UART_EndTransmit_IT(UART_HandleTypeDef *huart);

extern processor_handle_t urc_processor_hndl;
extern osEventFlagsId_t atc_xfer_event_group;
extern atc_processor_runtime_fsm_t atc_fsm;
//extern
//osThreadId_t socketTaskHandle;

struct tsm_UARTHandle externalComm;
struct tsm_UARTHandle internalComm;

//osThreadId_t serialCommExtTxTaskHandle;
//const osThreadAttr_t serialCommExtTxTask_attributes = {
//	.name = "serialCommExtTxTask",
//	.stack_size = 128 * 4,
//	.priority = (osPriority_t) osPriorityNormal
//};
osThreadId_t serialCommExtRxTaskHandle;
const osThreadAttr_t serialCommExtRxTask_attributes = {
	.name = "serialCommExtRxTask",
	.stack_size = 128 * 4,
	.priority = (osPriority_t) osPriorityNormal
};

osThreadId_t sercomCommonTxTaskHandle;
const osThreadAttr_t sercomCommonTxTask_attributes = {
	.name = "sercomCommonTxTask",
	.stack_size = 128*4,
	.priority = (osPriority_t) osPriorityNormal
};

//internal task
//osThreadId_t serialCommIntTxTaskHandle;
//const osThreadAttr_t serialCommIntTxTask_attributes = {
//		.name = "serialCommIntTxTask",
//		.stack_size = 128 * 4,
//		.priority = (osPriority_t) osPriorityNormal
//};

osThreadId_t serialCommIntRxTaskHandle;
const osThreadAttr_t serialCommIntRxTask_attributes = {
		.name = "serialCommIntRxTask",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal
};


#if 0
osThreadId_t serialCommTaskHandle;
const osThreadAttr_t serialCommTask_attributes = {
	.name = "serialCommTask",
	.stack_size = 128 * 4,
	.priority = (osPriority_t) osPriorityNormal
};
osThreadId_t serialCommTaskHandle;
const osThreadAttr_t serialCommTask_attributes = {
	.name = "serialCommTask",
	.stack_size = 128 * 4,
	.priority = (osPriority_t) osPriorityNormal
};
#endif

osEventFlagsId_t uart_xfer_event_group;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	struct tsm_UARTHandle *tsm_huart;
	if (&EXTERNAL_UART_HANDLE == huart){
		tsm_huart = &externalComm;
		if (externalComm.onTxCb != NULL) externalComm.onTxCb(NULL);
	}
	else if(&INTERNAL_UART_HANDLE == huart){
		tsm_huart = &internalComm;
		if (internalComm.onTxCb != NULL) internalComm.onTxCb(NULL);
	}
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	struct tsm_UARTHandle *tsm_huart;
	if (&EXTERNAL_UART_HANDLE == huart){
		tsm_huart = &externalComm;
		if (tsm_huart->onRxCb != NULL) tsm_huart->onRxCb(huart->hdmarx);
	}
	else if(&INTERNAL_UART_HANDLE == huart){
		tsm_huart = &internalComm;
		if (tsm_huart->onRxCb != NULL) tsm_huart->onRxCb(huart->hdmarx);
	}
}

static void serial_peripheral_setup(void){
//	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBuf, RxBuf_SIZE);
//	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

//	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuf, RxBuf_SIZE);
//	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
//	HAL_UART_ReceiverTimeout_Config(&huart1, 39);
//	HAL_UART_EnableReceiverTimeout(&huart1);

//	HAL_UART_ReceiverTimeout_Config(&huart2, 39);
//	HAL_UART_EnableReceiverTimeout(&huart2);

	memset(externalComm.tx_buff, 0x00, UART_BUFFER_SIZE);
	externalComm.state = 0; // TODO: Define states
	externalComm.huart = EXTERNAL_UART_HANDLE;
	externalComm.circularReceiver.head_pos = 0;
	externalComm.circularReceiver.tail_pos = 0;
	externalComm.onTxCb = extCommTxCompleted_cb;
	externalComm.onRxCb = extCommRxIncoming_cb;
	memset(externalComm.circularReceiver.rx_buff, 0x00, UART_BUFFER_SIZE);

#ifdef USE_INTERNAL_UART
	memset(internalComm.tx_buff, 0x00, UART_BUFFER_SIZE);
	internalComm.state = 0; // TODO: Define states
	internalComm.huart = INTERNAL_UART_HANDLE;
	internalComm.circularReceiver.head_pos = 0;
	internalComm.circularReceiver.tail_pos = 0;
	internalComm.onTxCb = intCommTxCompleted_cb;
	internalComm.onRxCb = intCommRxIncoming_cb;
	memset(internalComm.circularReceiver.rx_buff, 0x00, UART_BUFFER_SIZE);
#endif
}

static void intCommTxCompleted_cb(void *data){
	data_bridge_dir_out_notify();
}

static void intCommRxIncoming_cb(void *data){
	DMA_HandleTypeDef *dmaData = (DMA_HandleTypeDef *)data;
	struct uartDataQueue queueData;
	uint16_t dataLen;

	// To be replaced
	// Get data len
	dataLen = UART_BUFFER_SIZE - (dmaData->Instance->CNDTR);
	if ((UART_BUFFER_SIZE - dmaData->Instance->CNDTR) >= internalComm.circularReceiver.head_pos){
		dataLen -= internalComm.circularReceiver.head_pos;
	}
	if ((UART_BUFFER_SIZE - dmaData->Instance->CNDTR) <= internalComm.circularReceiver.head_pos){
		dataLen += (UART_BUFFER_SIZE - internalComm.circularReceiver.head_pos);
	}

	// Put queue
	queueData.data_start_pos = internalComm.circularReceiver.head_pos;
	queueData.len = dataLen;
	// Low Priority & without blocking
	osMessageQueuePut(internalComm.rxQueue, &queueData, 0U, 0U);

	internalComm.circularReceiver.head_pos = UART_BUFFER_SIZE - dmaData->Instance->CNDTR;
}

static void extCommTxCompleted_cb(void *data){
//	osEventFlagsSet(uart_xfer_event_group, UART_EXT_EVENT_DMA_TX_COMPLETE);

	// Temporary test only
	// Notify
	data_bridge_dir_out_notify();
}

static void extCommRxIncoming_cb(void *data){
	DMA_HandleTypeDef *dmaData = (DMA_HandleTypeDef *)data;
	struct uartDataQueue queueData;
	uint16_t dataLen;

	// To be replaced
	// Get data len
	dataLen = UART_BUFFER_SIZE - (dmaData->Instance->CNDTR);
	if ((UART_BUFFER_SIZE - dmaData->Instance->CNDTR) >= externalComm.circularReceiver.head_pos){
		dataLen -= externalComm.circularReceiver.head_pos;
	}
	if ((UART_BUFFER_SIZE - dmaData->Instance->CNDTR) <= externalComm.circularReceiver.head_pos){
		dataLen += (UART_BUFFER_SIZE - externalComm.circularReceiver.head_pos);
	}

	// Put queue
	queueData.data_start_pos = externalComm.circularReceiver.head_pos;
	queueData.len = dataLen;
	// Low Priority & without blocking
	osMessageQueuePut(externalComm.rxQueue, &queueData, 0U, 0U);

	externalComm.circularReceiver.head_pos = UART_BUFFER_SIZE - dmaData->Instance->CNDTR;
}

// Override UART Error Callback
//// RTO Interrupt is considered as error
//void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
//	struct tsm_UARTHandle *tsm_huart;
//
//	if (huart == &EXTERNAL_UART_HANDLE){
//		tsm_huart = &externalComm;
//#ifdef USE_INTERNAL_UART
//	}else if(huart == &INTERNAL_UART_HANDLE){
//		tsm_huart = &internalComm;
//#endif
//	}else{
//		// do something?
//		return;
//	}
//
//	if (tsm_huart->onRxCb != NULL) tsm_huart->onRxCb(huart->hdmarx);
//}

static void external_comm_recv_task(void *argument){
	while (1){
		process_as_urc();
//		osDelay(1000);
	}
}

static void internal_comm_recv_task(void *argument){
	while (1){
		internal_receive_urc();
//		osDelay(1000);
	}
}

static void common_tx_task(void *arg){
	// Wait for tx request signal
	uint32_t tx_flag;

//	const char enter[] = {0x0D, 0x0A};
	while (1){
		tx_flag = osEventFlagsWait(uart_xfer_event_group, UART_EVENT_TX_EXT_DIR_OUT_REQUESTED_E|UART_EVENT_TX_INT_DIR_OUT_REQUESTED_E, osFlagsWaitAny, osWaitForever);

//		atc_fsm.state = SERCOM_STATE_BRIDGE_E;
		// Loop Both UART's TX semaphore until full
		if (osOK == osSemaphoreAcquire(urc_processor_hndl.read_semaphore, 0)){
			if (tx_flag & UART_EVENT_TX_EXT_DIR_OUT_REQUESTED_E)
			{
				if (atc_fsm.state == SERCOM_STATE_BRIDGE_E) HAL_UART_Transmit_DMA(&INTERNAL_UART_HANDLE, urc_processor_hndl.curr_read->buff, urc_processor_hndl.curr_read->data_len);
				else HAL_UART_Transmit_DMA(&EXTERNAL_UART_HANDLE, urc_processor_hndl.curr_read->buff, urc_processor_hndl.curr_read->data_len);
			}

			if (tx_flag & UART_EVENT_TX_INT_DIR_OUT_REQUESTED_E)
			{
				if (atc_fsm.state == SERCOM_STATE_BRIDGE_E) HAL_UART_Transmit_DMA(&EXTERNAL_UART_HANDLE, urc_processor_hndl.curr_read->buff, urc_processor_hndl.curr_read->data_len);
				else HAL_UART_Transmit_DMA(&INTERNAL_UART_HANDLE, urc_processor_hndl.curr_read->buff, urc_processor_hndl.curr_read->data_len);
			}
		}
	}
}

void sercomm_init(void *args){
	// Test Only
	serial_peripheral_setup();
	atc_init();
	// Thread Synchronization Setup

	// Allocate and assign Queue Mem
	externalComm.rxQueue = osMessageQueueNew(MAX_UART_DATA_QUEUE, sizeof(struct uartDataQueue), NULL);
	externalComm.txQueue = osMessageQueueNew(MAX_UART_DATA_QUEUE, sizeof(struct uartDataQueue), NULL);

	internalComm.rxQueue = osMessageQueueNew(MAX_UART_DATA_QUEUE, sizeof(struct uartDataQueue), NULL);
	internalComm.txQueue = osMessageQueueNew(MAX_UART_DATA_QUEUE, sizeof(struct uartDataQueue), NULL);

	// Event Group
	uart_xfer_event_group = osEventFlagsNew(NULL);

	// To be removed
	//external
//	HAL_UART_Receive_DMA(&huart2, (uint8_t *)externalComm.circularReceiver.rx_buff, UART_BUFFER_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t *)internalComm.circularReceiver.rx_buff, UART_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
	//internal
//	HAL_UART_Receive_DMA(&huart1, (uint8_t *)internalComm.circularReceiver.rx_buff, UART_BUFFER_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)externalComm.circularReceiver.rx_buff, UART_BUFFER_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

	// Start individual threads
	//external
	serialCommExtRxTaskHandle = osThreadNew(external_comm_recv_task, NULL, &serialCommExtRxTask_attributes);
	//internal
	serialCommIntRxTaskHandle = osThreadNew(internal_comm_recv_task, NULL, &serialCommIntRxTask_attributes);
	sercomCommonTxTaskHandle = osThreadNew(common_tx_task, NULL, &sercomCommonTxTask_attributes);


}




static HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart)
{
  uint8_t  *pdata8bits;
  uint16_t *pdata16bits;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      pdata8bits  = NULL;
      pdata16bits = (uint16_t *) huart->pRxBuffPtr;
      *pdata16bits = (uint16_t)(huart->Instance->DR & (uint16_t)0x01FF);
      huart->pRxBuffPtr += 2U;
    }
    else
    {
      pdata8bits = (uint8_t *) huart->pRxBuffPtr;
      pdata16bits  = NULL;

      if ((huart->Init.WordLength == UART_WORDLENGTH_9B) || ((huart->Init.WordLength == UART_WORDLENGTH_8B) && (huart->Init.Parity == UART_PARITY_NONE)))
      {
        *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
      }
      else
      {
        *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F);
      }
      huart->pRxBuffPtr += 1U;
    }

    if (--huart->RxXferCount == 0U)
    {
      /* Disable the UART Data Register not empty Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

      /* Disable the UART Parity Error Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

      /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      /* Initialize type of RxEvent to Transfer Complete */
      huart->RxEventType = HAL_UART_RXEVENT_TC;

      /* Check current reception Mode :
         If Reception till IDLE event has been selected : */
      if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
      {
        /* Set reception type to Standard */
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        /* Disable IDLE interrupt */
        ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);

        /* Check if IDLE flag is set */
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
        {
          /* Clear IDLE flag in ISR */
          __HAL_UART_CLEAR_IDLEFLAG(huart);
        }

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx Event callback*/
        huart->RxEventCallback(huart, huart->RxXferSize);
#else
        /*Call legacy weak Rx Event callback*/
        HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }
      else
      {
        /* Standard reception API called */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx complete callback*/
        huart->RxCpltCallback(huart);
#else
        /*Call legacy weak Rx complete callback*/
        HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }

      return HAL_OK;
    }
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  huart->RxXferCount = 0x00U;
  huart->TxXferCount = 0x00U;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered error callback*/
  huart->ErrorCallback(huart);
#else
  /*Call legacy weak error callback*/
  HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

static void UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* In case of reception waiting for IDLE event, disable also the IDLE IE interrupt source */
  if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
  {
    ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);
  }

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;
  huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
}

static HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart)
{
  const uint16_t *tmp;

  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      tmp = (const uint16_t *) huart->pTxBuffPtr;
      huart->Instance->DR = (uint16_t)(*tmp & (uint16_t)0x01FF);
      huart->pTxBuffPtr += 2U;
    }
    else
    {
      huart->Instance->DR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0x00FF);
    }

    if (--huart->TxXferCount == 0U)
    {
      /* Disable the UART Transmit Data Register Empty Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

      /* Enable the UART Transmit Complete Interrupt */
      __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
    }
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}


static HAL_StatusTypeDef UART_EndTransmit_IT(UART_HandleTypeDef *huart)
{
  /* Disable the UART Transmit Complete Interrupt */
  __HAL_UART_DISABLE_IT(huart, UART_IT_TC);

  /* Tx process is ended, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  /*Call registered Tx complete callback*/
  huart->TxCpltCallback(huart);
#else
  /*Call legacy weak Tx complete callback*/
  HAL_UART_TxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

  return HAL_OK;
}

void TSM_UART_IRQ_Handler(UART_HandleTypeDef *huart)
{
  uint32_t isrflags   = READ_REG(huart->Instance->SR);
  uint32_t cr1its     = READ_REG(huart->Instance->CR1);
  uint32_t cr3its     = READ_REG(huart->Instance->CR3);
  uint32_t errorflags = 0x00U;
  uint32_t dmarequest = 0x00U;

  /* If no error occurs */
  errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
  if (errorflags == RESET)
  {
    /* UART in mode Receiver -------------------------------------------------*/
    if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
      UART_Receive_IT(huart);
      return;
    }
  }

  /* If some errors occur */
  if ((errorflags != RESET) && (((cr3its & USART_CR3_EIE) != RESET)
                                || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
  {
    /* UART parity error interrupt occurred ----------------------------------*/
    if (((isrflags & USART_SR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
    {
      huart->ErrorCode |= HAL_UART_ERROR_PE;
    }

    /* UART noise error interrupt occurred -----------------------------------*/
    if (((isrflags & USART_SR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
    {
      huart->ErrorCode |= HAL_UART_ERROR_NE;
    }

    /* UART frame error interrupt occurred -----------------------------------*/
    if (((isrflags & USART_SR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
    {
      huart->ErrorCode |= HAL_UART_ERROR_FE;
    }

    /* UART Over-Run interrupt occurred --------------------------------------*/
    if (((isrflags & USART_SR_ORE) != RESET) && (((cr1its & USART_CR1_RXNEIE) != RESET)
                                                 || ((cr3its & USART_CR3_EIE) != RESET)))
    {
      huart->ErrorCode |= HAL_UART_ERROR_ORE;
    }

    /* Call UART Error Call back function if need be --------------------------*/
    if (huart->ErrorCode != HAL_UART_ERROR_NONE)
    {
      /* UART in mode Receiver -----------------------------------------------*/
      if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
      {
        UART_Receive_IT(huart);
      }

      /* If Overrun error occurs, or if any error occurs in DMA mode reception,
         consider error as blocking */
      dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR);
      if (((huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET) || dmarequest)
      {
        /* Blocking error : transfer is aborted
           Set the UART state ready to be able to start again the process,
           Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
        UART_EndRxTransfer(huart);

        /* Disable the UART DMA Rx request if enabled */
        if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
        {
          ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

          /* Abort the UART DMA Rx channel */
          if (huart->hdmarx != NULL)
          {
            /* Set the UART DMA Abort callback :
               will lead to call HAL_UART_ErrorCallback() at end of DMA abort procedure */
            huart->hdmarx->XferAbortCallback = UART_DMAAbortOnError;
            if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
            {
              /* Call Directly XferAbortCallback function in case of error */
              huart->hdmarx->XferAbortCallback(huart->hdmarx);
            }
          }
          else
          {
            /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
            /*Call registered error callback*/
            huart->ErrorCallback(huart);
#else
            /*Call legacy weak error callback*/
            HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
          }
        }
        else
        {
          /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
          /*Call registered error callback*/
          huart->ErrorCallback(huart);
#else
          /*Call legacy weak error callback*/
          HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
        }
      }
      else
      {
        /* Non Blocking error : transfer could go on.
           Error is notified to user through user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered error callback*/
        huart->ErrorCallback(huart);
#else
        /*Call legacy weak error callback*/
        HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

        huart->ErrorCode = HAL_UART_ERROR_NONE;
      }
    }
    return;
  } /* End if some error occurs */

  /* Check current reception Mode :
     If Reception till IDLE event has been selected : */
  if ((huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
      && ((isrflags & USART_SR_IDLE) != 0U)
      && ((cr1its & USART_SR_IDLE) != 0U))
  {
    __HAL_UART_CLEAR_IDLEFLAG(huart);

    /* Check if DMA mode is enabled in UART */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR))
    {
      /* DMA mode enabled */
      /* Check received length : If all expected data are received, do nothing,
         (DMA cplt callback will be called).
         Otherwise, if at least one data has already been received, IDLE event is to be notified to user */
      uint16_t nb_remaining_rx_data = (uint16_t) __HAL_DMA_GET_COUNTER(huart->hdmarx);
      if ((nb_remaining_rx_data > 0U)
          && (nb_remaining_rx_data < huart->RxXferSize))
      {
        /* Reception is not complete */
        huart->RxXferCount = nb_remaining_rx_data;

        /* In Normal mode, end DMA xfer and HAL UART Rx process*/
        if (huart->hdmarx->Init.Mode != DMA_CIRCULAR)
        {
          /* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
          ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE);
          ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

          /* Disable the DMA transfer for the receiver request by resetting the DMAR bit
             in the UART CR3 register */
          ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

          /* At end of Rx process, restore huart->RxState to Ready */
          huart->RxState = HAL_UART_STATE_READY;
          huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

          ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);

          /* Last bytes received, so no need as the abort is immediate */
          (void)HAL_DMA_Abort(huart->hdmarx);
        }

        /* Initialize type of RxEvent that correspond to RxEvent callback execution;
        In this case, Rx Event type is Idle Event */
        huart->RxEventType = HAL_UART_RXEVENT_IDLE;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx Event callback*/
        huart->RxEventCallback(huart, (huart->RxXferSize - huart->RxXferCount));
#else
        /*Call legacy weak Rx Event callback*/
        HAL_UARTEx_RxEventCallback(huart, (huart->RxXferSize - huart->RxXferCount));
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }
      return;
    }
    else
    {
      /* DMA mode not enabled */
      /* Check received length : If all expected data are received, do nothing.
         Otherwise, if at least one data has already been received, IDLE event is to be notified to user */
      uint16_t nb_rx_data = huart->RxXferSize - huart->RxXferCount;
      if ((huart->RxXferCount > 0U)
          && (nb_rx_data > 0U))
      {
        /* Disable the UART Parity Error Interrupt and RXNE interrupts */
        ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

        /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
        ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

        /* Rx process is completed, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);

        /* Initialize type of RxEvent that correspond to RxEvent callback execution;
           In this case, Rx Event type is Idle Event */
        huart->RxEventType = HAL_UART_RXEVENT_IDLE;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx complete callback*/
        huart->RxEventCallback(huart, nb_rx_data);
#else
        /*Call legacy weak Rx Event callback*/
        HAL_UARTEx_RxEventCallback(huart, nb_rx_data);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
      }
      return;
    }
  }

  /* UART in mode Transmitter ------------------------------------------------*/
  if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
  {
    UART_Transmit_IT(huart);
    return;
  }

  /* UART in mode Transmitter end --------------------------------------------*/
  if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
  {
    UART_EndTransmit_IT(huart);
    return;
  }
}

