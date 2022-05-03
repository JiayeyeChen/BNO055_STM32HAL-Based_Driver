/** @file 	bno055_imu.h
 *	@brief 	Header file for the BNO055 IMU.
 *  @author Jia ye
 */
 
#ifndef _BNO055_H
#define _BNO055_H

/* Include -------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"

/* Defines -------------------------------------------------------------------*/
//Defines related to BNO055
#define               REG_OPR_MODE               0x3D
#define               REG_LIA_X_LSB              0x28
#define               REG_GYR_X_LSB              0x14
#define               REG_QUAT_W_LSB             0x20
#define               REG_CALIB_STAT             0x35
#define               REG_MAG_RADIUS_MSB         0x6A
#define               REG_MAG_RADIUS_LSB         0x69
#define               REG_ACC_RADIUS_MSB         0x68
#define               REG_ACC_RADIUS_LSB         0x67
#define               REG_GYR_OFFSET_Z_MSB       0x66
#define               REG_GYR_OFFSET_Z_LSB       0x65
#define               REG_GYR_OFFSET_Y_MSB       0x64
#define               REG_GYR_OFFSET_Y_LSB       0x63
#define               REG_GYR_OFFSET_X_MSB       0x62
#define               REG_GYR_OFFSET_X_LSB       0x61
#define               REG_MAG_OFFSET_Z_MSB       0x60
#define               REG_MAG_OFFSET_Z_LSB       0x5F
#define               REG_MAG_OFFSET_Y_MSB       0x5E
#define               REG_MAG_OFFSET_Y_LSB       0x5D
#define               REG_MAG_OFFSET_X_MSB       0x5C
#define               REG_MAG_OFFSET_X_LSB       0x5B
#define               REG_ACC_OFFSET_Z_MSB       0x5A
#define               REG_ACC_OFFSET_Z_LSB       0x59
#define               REG_ACC_OFFSET_Y_MSB       0x58
#define               REG_ACC_OFFSET_Y_LSB       0x57
#define               REG_ACC_OFFSET_X_MSB       0x56
#define               REG_ACC_OFFSET_X_LSB       0x55
#define								REG_SYS_TRIGGER						 0x3F

#define               MODE_ACCGYRO               0x05
#define               MODE_ACCONLY               0x01
#define               MODE_NDOF                  0x0C
#define               MODE_IMU                   0x08
#define               MODE_CONFIG                0x00
#define								MODE_UNKNOWN							 0xFF

#define               CALIBPROFILE_MAG_RADIUS_MSB      0x01
#define               CALIBPROFILE_MAG_RADIUS_LSB      0xE5
#define               CALIBPROFILE_ACC_RADIUS_MSB      0x03
#define               CALIBPROFILE_ACC_RADIUS_LSB      0xE8
#define               CALIBPROFILE_GYR_OFFSET_Z_MSB    0xFF
#define               CALIBPROFILE_GYR_OFFSET_Z_LSB    0xFF
#define               CALIBPROFILE_GYR_OFFSET_Y_MSB    0xFF
#define               CALIBPROFILE_GYR_OFFSET_Y_LSB    0xFD
#define               CALIBPROFILE_GYR_OFFSET_X_MSB    0xFF
#define               CALIBPROFILE_GYR_OFFSET_X_LSB    0xFE
#define               CALIBPROFILE_MAG_OFFSET_Z_MSB    0x00
#define               CALIBPROFILE_MAG_OFFSET_Z_LSB    0x61
#define               CALIBPROFILE_MAG_OFFSET_Y_MSB    0xFE
#define               CALIBPROFILE_MAG_OFFSET_Y_LSB    0xEB
#define               CALIBPROFILE_MAG_OFFSET_X_MSB    0xFE
#define               CALIBPROFILE_MAG_OFFSET_X_LSB    0xC3
#define               CALIBPROFILE_ACC_OFFSET_Z_MSB    0xFF
#define               CALIBPROFILE_ACC_OFFSET_Z_LSB    0xF0
#define               CALIBPROFILE_ACC_OFFSET_Y_MSB    0x00
#define               CALIBPROFILE_ACC_OFFSET_Y_LSB    0x14
#define               CALIBPROFILE_ACC_OFFSET_X_MSB    0x00
#define               CALIBPROFILE_ACC_OFFSET_X_LSB    0x0D

//Defines related to MCU
#define IMU_RST_Pin GPIO_PIN_5
#define IMU_RST_GPIO_Port GPIOB

enum BNO055_Error
{
	BNO055_ERROR_NONE,
	BNO055_ERROR_I2C_LINE_ANORMAL,
	BNO055_ERROR_IMU_WRONG_OPERATION_MODE
};

enum BNO055_SubTask
{
	BNO055_SubTask_READ_LA,
	BNO055_SubTask_READ_GY,
	BNO055_SubTask_READ_OPERATION_MODE,
	BNO055_SubTask_Reset_IMU,
	BNO055_SubTask_Set_Calibration_Data,
	BNO055_SubTask_Set_9DOF_Mode,
	BNO055_SubTask_Read_Calib_Status,
	BNO055_SubTask_Read_SYS_TRIGGER_Register,
	BNO055_SubTask_Set_External_Clock_Source,
	BNO055_SubTask_NONE
};

enum BNO055_Task
{
	BNO055_Task_READ_LA_GY_OPERATION_MODE_CABLIB_STATUS,
	BNO055_Task_INITIALIZATION,
	BNO055_Task_READ_AND_SET_EXTERNAL_CLOCK_SOURCE,
	BNO055_Task_NONE,
  BNO055_Task_GYRO_ONLY
};

union BNO055_union_int16u8 {
	int16_t int16;
	uint8_t uint8[2];
};

typedef struct _CalibProfTypdef
{
	uint8_t MAG_RADIUS_MSB;       uint8_t MAG_RADIUS_LSB;
	uint8_t ACC_RADIUS_MSB;       uint8_t ACC_RADIUS_LSB;
	uint8_t GYR_OFFSET_Z_MSB;     uint8_t GYR_OFFSET_Z_LSB;
	uint8_t GYR_OFFSET_Y_MSB;     uint8_t GYR_OFFSET_Y_LSB;
	uint8_t GYR_OFFSET_X_MSB;     uint8_t GYR_OFFSET_X_LSB;
	uint8_t MAG_OFFSET_Z_MSB;     uint8_t MAG_OFFSET_Z_LSB;
	uint8_t MAG_OFFSET_Y_MSB;     uint8_t MAG_OFFSET_Y_LSB;
	uint8_t MAG_OFFSET_X_MSB;     uint8_t MAG_OFFSET_X_LSB;
	uint8_t ACC_OFFSET_Z_MSB;     uint8_t ACC_OFFSET_Z_LSB;
	uint8_t ACC_OFFSET_Y_MSB;     uint8_t ACC_OFFSET_Y_LSB;
	uint8_t ACC_OFFSET_X_MSB;     uint8_t ACC_OFFSET_X_LSB;
}CalibProfTypdef;

typedef struct
{
	union BNO055_union_int16u8 liaccX;
	union BNO055_union_int16u8 liaccY;
	union BNO055_union_int16u8 liaccZ;
	union BNO055_union_int16u8 gyroX;
	union BNO055_union_int16u8 gyroY;
	union BNO055_union_int16u8 gyroZ;
	union BNO055_union_int16u8 Quat[4];
}BNO055_Data_Struct;

typedef struct
{
	uint32_t lastAllTaskFinishTime;
	uint32_t timeOutDuration;
	uint8_t ifDataReadTaskFinished;
	uint8_t timeOutCount;
}BNO055_TimeOut_Struct;

typedef struct
{
	I2C_HandleTypeDef*				hi2c;
	uint16_t 									i2cAddr;
	uint8_t										txBuf[2];
	uint8_t										rxBuf[22];
	uint8_t										operationModeReal;
	uint8_t										calibStatus;
	uint8_t										deadCount;
	CalibProfTypdef 					calibData;
	BNO055_Data_Struct				data;
	BNO055_TimeOut_Struct			timeout;
	GPIO_TypeDef*							resetPinPort;
	uint16_t									resetPin;
	enum BNO055_SubTask				subTask;
	enum BNO055_Task					curTask;
	enum BNO055_Task					goalTask;
	enum BNO055_Error					error;
	HAL_StatusTypeDef					isDevReady;
	uint8_t										isNewData;
	uint8_t										isInitialized;
	uint8_t										systemTriggerRegister;
}BNO055_Struct;

void BNO055_Reset(BNO055_Struct* hBNO055);
void BNO055_MCU_Init(void);
uint8_t BNO055_IMU_Init(BNO055_Struct* hBNO055);
void BNO055_SetOperationMode(BNO055_Struct* hBNO055, uint8_t modeToSet);
void BNO055_ReadOperationMode(BNO055_Struct* hBNO055);
void BNO055_Read_LiAc_Request(BNO055_Struct* hBNO055);
void BNO055_Get_LiAc(BNO055_Struct* hBNO055);
void BNO055_Read_Gyro_Request(BNO055_Struct* hBNO055);
void BNO055_Get_Gyro(BNO055_Struct* hBNO055);
void BNO055_Read_Operation_Mode_Request(BNO055_Struct* hBNO055);
void BNO055_Get_Operation_Mode(BNO055_Struct* hBNO055);
void BNO055_SetPrecalibProfile(BNO055_Struct* hBNO055);
void BNO055_ReadCalibrationStatus(BNO055_Struct* hBNO055);
void BNO055_Read_Register_Request(BNO055_Struct* hBNO055, uint8_t regAddr, uint16_t size);
void BNO055_Read_LiAc_Gyro(BNO055_Struct* hBNO055);
void BNO055_ReadCalibProf(BNO055_Struct* hBNO055);
void BNO055_Force_to_NDOF_Mode(BNO055_Struct* hBNO055);
void BNO055_I2C_Init(void);
void BNO055_Task_Manager(BNO055_Struct* hBNO055, enum BNO055_Task taskToDo);
void BNO055_Time_Out_Reset(BNO055_Struct* hBNO055);
void BNO055_Set_External_Clock_Source(BNO055_Struct* hBNO055);


extern BNO055_Struct hBNO055;

#endif
