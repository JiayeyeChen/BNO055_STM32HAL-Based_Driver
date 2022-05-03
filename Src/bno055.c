/** @file 	bno055_imu.c
 *	@brief 	Source file for the BNO055 IMU.
 *  @author Jia ye
 */
 
#include "bno055.h"

BNO055_Struct hBNO055;
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

//for testing//
uint8_t i2caborttest = 0;
uint8_t i2crxcplttest = 0;
uint8_t i2ctxcplttest = 0;
uint8_t errorcallbacktest = 0;
uint32_t timeouttest;
///////////////
void BNO055_MCU_Init(void)
{
	/* BNO055 handle initialization. */
	hBNO055.i2cAddr = 0x0050;
	hBNO055.hi2c = &hi2c1;
	hBNO055.error = BNO055_ERROR_NONE;
	hBNO055.operationModeReal = MODE_CONFIG;
	hBNO055.resetPinPort = IMU_RST_GPIO_Port;
	hBNO055.resetPin = IMU_RST_Pin;
	hBNO055.calibStatus = 0x00;
	hBNO055.subTask = BNO055_SubTask_Reset_IMU;
	hBNO055.curTask = BNO055_Task_INITIALIZATION;
	hBNO055.deadCount = 0;
	hBNO055.timeout.timeOutCount = 0;
	hBNO055.isNewData = 0;
	hBNO055.isInitialized = 0;
	hBNO055.systemTriggerRegister = 0xFF;
	
	/* MCU I2C module initialization. */
	__HAL_RCC_DMA1_CLK_ENABLE();
	HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	__HAL_RCC_I2C1_CLK_ENABLE();
	
	hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;//I2C_DUTYCYCLE_16_9;//
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
	
  
	/* I2C1 DMA Init */
	/* I2C1_RX Init */
	hdma_i2c1_rx.Instance = DMA1_Channel7;
	hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_i2c1_rx);
	__HAL_LINKDMA(&hi2c1,hdmarx,hdma_i2c1_rx);
	/* I2C1_TX Init */
	hdma_i2c1_tx.Instance = DMA1_Channel6;
	hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_i2c1_tx);
	__HAL_LINKDMA(&hi2c1,hdmatx,hdma_i2c1_tx);
		
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

	/* MCU IMU reset pin initialization. */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct.Pin = IMU_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IMU_RST_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, GPIO_PIN_SET);
}

uint8_t BNO055_IMU_Init(BNO055_Struct* hBNO055)
{
	BNO055_Reset(hBNO055);
	if (HAL_I2C_IsDeviceReady(&hi2c1, hBNO055->i2cAddr, 50, 10) == HAL_OK)
	{
		BNO055_SetPrecalibProfile(hBNO055);
		hBNO055->operationModeReal = MODE_CONFIG;
		hBNO055->error = BNO055_ERROR_NONE;
		return 0;
	}
	else
	{
		hBNO055->error = BNO055_ERROR_I2C_LINE_ANORMAL;
		return 1;
	}
}

void BNO055_ReadOperationMode(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_OPR_MODE, 1);
	DWT_Delay_us(50);
	hBNO055->operationModeReal = hBNO055->rxBuf[0] &= 0x0F;
}

void BNO055_SetOperationMode(BNO055_Struct* hBNO055, uint8_t modeToSet)
{
	HAL_I2C_Mem_Write_DMA(hBNO055->hi2c, hBNO055->i2cAddr, (uint16_t)REG_OPR_MODE, I2C_MEMADD_SIZE_8BIT, &modeToSet, 1);
}

void BNO055_Set_External_Clock_Source(BNO055_Struct* hBNO055)
{
	uint8_t temp = 0x80;
	HAL_I2C_Mem_Write_DMA(hBNO055->hi2c, hBNO055->i2cAddr, (uint16_t)REG_SYS_TRIGGER, I2C_MEMADD_SIZE_8BIT, &temp, 1);
}

void BNO055_SetPrecalibProfile(BNO055_Struct* hBNO055)
{
	uint8_t calibBuf[22];
	uint8_t ptr = 0;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_X_LSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_X_MSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_Y_LSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_Y_MSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_Z_LSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_OFFSET_Z_MSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_X_LSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_X_MSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_Y_LSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_Y_MSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_Z_LSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_OFFSET_Z_MSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_X_LSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_X_MSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_Y_LSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_Y_MSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_Z_LSB;
	calibBuf[ptr++] = CALIBPROFILE_GYR_OFFSET_Z_MSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_RADIUS_LSB;
	calibBuf[ptr++] = CALIBPROFILE_ACC_RADIUS_MSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_RADIUS_LSB;
	calibBuf[ptr++] = CALIBPROFILE_MAG_RADIUS_MSB;
	HAL_I2C_Mem_Write_DMA(hBNO055->hi2c, hBNO055->i2cAddr, REG_ACC_OFFSET_X_LSB, I2C_MEMADD_SIZE_8BIT, calibBuf, 22);
}

void BNO055_Reset(BNO055_Struct* hBNO055)
{
	HAL_GPIO_WritePin(hBNO055->resetPinPort, hBNO055->resetPin, GPIO_PIN_SET);
	HAL_Delay(40);
	HAL_GPIO_WritePin(hBNO055->resetPinPort, hBNO055->resetPin, GPIO_PIN_RESET);
	HAL_Delay(40);//at least 20ns by datasheet
	HAL_GPIO_WritePin(hBNO055->resetPinPort, hBNO055->resetPin, GPIO_PIN_SET);
	HAL_Delay(600);//essential to be a large time, otherwise the IMU can be never reset. Havent tried a smaller ones but 800 works stably.
}

void BNO055_ReadCalibrationStatus(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_CALIB_STAT, 1);
	hBNO055->calibStatus = hBNO055->rxBuf[0];
}



void BNO055_Read_Register_Request(BNO055_Struct* hBNO055, uint8_t regAddr, uint16_t size)
{
	HAL_I2C_Mem_Read_DMA(hBNO055->hi2c, hBNO055->i2cAddr, (uint16_t)regAddr, I2C_MEMADD_SIZE_8BIT, &hBNO055->rxBuf[0], size);
}

void BNO055_Read_LiAc_Gyro(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_LIA_X_LSB, 6);
	uint8_t ptr = 0;
	hBNO055->data.liaccX.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccX.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccY.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccY.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccZ.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccZ.uint8[1] = hBNO055->rxBuf[ptr++];
	
	BNO055_Read_Register_Request(hBNO055, REG_GYR_X_LSB, 6);
	ptr = 0;
	hBNO055->data.gyroX.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroX.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroY.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroY.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroZ.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroZ.uint8[1] = hBNO055->rxBuf[ptr++];
}

void BNO055_ReadCalibProf(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_ACC_OFFSET_X_LSB, 22);
	uint8_t ptr = 0;
	hBNO055->calibData.ACC_OFFSET_X_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_OFFSET_X_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_OFFSET_Y_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_OFFSET_Y_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_OFFSET_Z_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_OFFSET_Z_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_X_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_X_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_Y_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_Y_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_Z_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_OFFSET_Z_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_X_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_X_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_Y_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_Y_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_Z_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.GYR_OFFSET_Z_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_RADIUS_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.ACC_RADIUS_MSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_RADIUS_LSB = hBNO055->rxBuf[ptr++];
	hBNO055->calibData.MAG_RADIUS_MSB = hBNO055->rxBuf[ptr++];
}

void BNO055_Force_to_NDOF_Mode(BNO055_Struct* hBNO055)
{
	while (hBNO055->operationModeReal != MODE_NDOF)
	{
		BNO055_SetOperationMode(hBNO055, MODE_NDOF);
		BNO055_ReadOperationMode(hBNO055);
	}
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C2)
	{
		i2caborttest++;
	}
}

void BNO055_I2C_Init(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	__HAL_RCC_I2C1_CLK_ENABLE();
	
	hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;//I2C_DUTYCYCLE_16_9;//
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
	
  
	/* I2C1 DMA Init */
	/* I2C1_RX Init */
	hdma_i2c1_rx.Instance = DMA1_Channel7;
	hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_i2c1_rx);
	__HAL_LINKDMA(&hi2c1,hdmarx,hdma_i2c1_rx);
	/* I2C1_TX Init */
	hdma_i2c1_tx.Instance = DMA1_Channel6;
	hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_i2c1_tx);
	__HAL_LINKDMA(&hi2c1,hdmatx,hdma_i2c1_tx);
		
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	HAL_NVIC_SetPriority(I2C1_ER_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}

void BNO055_Read_LiAc_Request(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_LIA_X_LSB, 6);
}

void BNO055_Get_LiAc(BNO055_Struct* hBNO055)
{
	uint8_t ptr = 0;
	hBNO055->data.liaccX.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccX.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccY.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccY.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccZ.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.liaccZ.uint8[1] = hBNO055->rxBuf[ptr++];
}

void BNO055_Read_Gyro_Request(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_GYR_X_LSB, 6);
}

void BNO055_Get_Gyro(BNO055_Struct* hBNO055)
{
	uint8_t ptr = 0;
	hBNO055->data.gyroX.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroX.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroY.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroY.uint8[1] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroZ.uint8[0] = hBNO055->rxBuf[ptr++];
	hBNO055->data.gyroZ.uint8[1] = hBNO055->rxBuf[ptr++];
}

void BNO055_Read_Operation_Mode_Request(BNO055_Struct* hBNO055)
{
	BNO055_Read_Register_Request(hBNO055, REG_OPR_MODE, 1);
}

void BNO055_Get_Operation_Mode(BNO055_Struct* hBNO055)
{
	hBNO055->operationModeReal = hBNO055->rxBuf[0] &= 0x0F;
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hBNO055.curTask == BNO055_Task_READ_LA_GY_OPERATION_MODE_CABLIB_STATUS)
	{
		if (hBNO055.subTask == BNO055_SubTask_READ_LA)
		{
			BNO055_Get_LiAc(&hBNO055);
			hBNO055.subTask = BNO055_SubTask_READ_GY;
			BNO055_Read_Gyro_Request(&hBNO055);
		}
		else if (hBNO055.subTask == BNO055_SubTask_READ_GY)
		{
			BNO055_Get_Gyro(&hBNO055);
			hBNO055.subTask = BNO055_SubTask_READ_OPERATION_MODE;
			BNO055_Read_Operation_Mode_Request(&hBNO055);
		}
		else if (hBNO055.subTask == BNO055_SubTask_READ_OPERATION_MODE)
		{
			BNO055_Get_Operation_Mode(&hBNO055);
			hBNO055.subTask = BNO055_SubTask_Read_Calib_Status;
			BNO055_Read_Register_Request(&hBNO055, REG_CALIB_STAT, 1);
		}
		else if (hBNO055.subTask == BNO055_SubTask_Read_Calib_Status)
		{
			hBNO055.calibStatus = hBNO055.rxBuf[0];
			hBNO055.subTask = BNO055_SubTask_READ_LA;
			hBNO055.timeout.ifDataReadTaskFinished = 1;
			hBNO055.timeout.lastAllTaskFinishTime = HAL_GetTick();
			hBNO055.isNewData =1;
		}
	}
	else if (hBNO055.curTask == BNO055_Task_READ_AND_SET_EXTERNAL_CLOCK_SOURCE)
	{
		if (hBNO055.subTask == BNO055_SubTask_Read_SYS_TRIGGER_Register)
		{
			hBNO055.systemTriggerRegister = hBNO055.rxBuf[0];
			if (hBNO055.systemTriggerRegister != 0x80)
			{
				hBNO055.subTask = BNO055_SubTask_Set_External_Clock_Source;
				BNO055_Set_External_Clock_Source(&hBNO055);
			}
		}
	}
	
}

void BNO055_Task_Manager(BNO055_Struct* hBNO055, enum BNO055_Task taskToDo)
{
	hBNO055->goalTask = taskToDo;
	
	if (hBNO055->curTask == BNO055_Task_READ_LA_GY_OPERATION_MODE_CABLIB_STATUS)
	{
		hBNO055->timeout.timeOutDuration = HAL_GetTick() - hBNO055->timeout.lastAllTaskFinishTime;
		
		//reset IMU when time out detected
		if (hBNO055->timeout.timeOutDuration > 100)
		{
			hBNO055->timeout.timeOutCount++;
			BNO055_Time_Out_Reset(hBNO055);
			return;
		}
		hBNO055->subTask = BNO055_SubTask_READ_LA;
		BNO055_Read_LiAc_Request(hBNO055);
		hBNO055->timeout.ifDataReadTaskFinished = 0;
	}
	else if (hBNO055->curTask == BNO055_Task_READ_AND_SET_EXTERNAL_CLOCK_SOURCE)
	{
		hBNO055->subTask = BNO055_SubTask_Read_SYS_TRIGGER_Register;
		BNO055_Read_Register_Request(hBNO055, REG_SYS_TRIGGER, 1);
	}
	
	/////////////////////////////////////////////////////////////////////
	else if (hBNO055->curTask == BNO055_Task_INITIALIZATION)
	{
		if (hBNO055->subTask == BNO055_SubTask_Reset_IMU)
		{
			BNO055_Reset(hBNO055);
			HAL_Delay(40);
			hBNO055->subTask = BNO055_SubTask_Set_Calibration_Data;
		}
		else if (hBNO055->subTask == BNO055_SubTask_Set_Calibration_Data)
			BNO055_SetPrecalibProfile(hBNO055);
		else if (hBNO055->subTask == BNO055_SubTask_Set_9DOF_Mode)
			BNO055_SetOperationMode(hBNO055, MODE_NDOF);
	}
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hBNO055.curTask == BNO055_Task_INITIALIZATION)
	{
		if (hBNO055.subTask == BNO055_SubTask_Set_Calibration_Data)
		{
			hBNO055.subTask = BNO055_SubTask_Set_9DOF_Mode;
		}
		else if (hBNO055.subTask == BNO055_SubTask_Set_9DOF_Mode)
		{
			hBNO055.curTask = hBNO055.goalTask;
			hBNO055.timeout.ifDataReadTaskFinished = 1;
			hBNO055.timeout.lastAllTaskFinishTime = HAL_GetTick();
			hBNO055.isInitialized = 1;
		}
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	errorcallbacktest++;
}

void BNO055_Time_Out_Reset(BNO055_Struct* hBNO055)
{
	//reset MCU I2C module
	HAL_I2C_DeInit(hBNO055->hi2c);
	HAL_DMA_DeInit(&hdma_i2c1_rx);
	HAL_DMA_DeInit(&hdma_i2c1_tx);
	
	__HAL_RCC_I2C1_FORCE_RESET();
	DWT_Delay_us(200);
	__HAL_RCC_I2C1_RELEASE_RESET();

	hBNO055->hi2c->Instance->CR1 |= 0x8000;
	DWT_Delay_us(200);
	hBNO055->hi2c->Instance->CR1 &= 0x7FFF;
	BNO055_I2C_Init();
	//reset and reconfigure IMU
	hBNO055->curTask = BNO055_Task_INITIALIZATION;
	hBNO055->subTask = BNO055_SubTask_Reset_IMU;
	hBNO055->timeout.ifDataReadTaskFinished = 1;
	hBNO055->operationModeReal = MODE_UNKNOWN;
	hBNO055->isNewData = 0;
	hBNO055->isInitialized = 0;
}
