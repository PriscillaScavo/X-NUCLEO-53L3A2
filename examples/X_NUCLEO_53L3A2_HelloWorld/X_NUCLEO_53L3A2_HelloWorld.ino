/**
 ******************************************************************************
 * @file    X_NUCLEO_53L3A2_HelloWorld.ino
 * @author  SRA
 * @version V1.0.0
 * @date    30 July 2020
 * @brief   Arduino test application for the STMicrolectronics X-NUCLEO-53L3A2
 *          proximity sensor expansion board based on FlightSense.
 *          This application makes use of C++ classes obtained from the C
 *          components' drivers.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2020 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include <Wire.h>
#include <vl53lx_x_nucleo_53l3a2_class.h>
#include <stmpe1600_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define DEV_I2C Wire
#define SerialPort Serial

/* Please uncomment the line below if you want also to use the satellites */
//#define SATELLITES_MOUNTED

// Components.
STMPE1600DigiOut *xshutdown_top;
VL53LX_X_NUCLEO_53L3A2 *sensor_vl53lx_top;
#ifdef SATELLITES_MOUNTED
  STMPE1600DigiOut *xshutdown_left;
  VL53LX_X_NUCLEO_53L3A2 *sensor_vl53lx_left;
  STMPE1600DigiOut *xshutdown_right;
  VL53LX_X_NUCLEO_53L3A2 *sensor_vl53lx_right;
#endif

/* Setup ---------------------------------------------------------------------*/

void setup()
{
  // Led.
  pinMode(13, OUTPUT);

  // Initialize serial for output.
  SerialPort.begin(115200);
  SerialPort.println("Starting...");

  // Initialize I2C bus.
  DEV_I2C.begin();

  // Create VL53LX top component.
  xshutdown_top = new STMPE1600DigiOut(&DEV_I2C, GPIO_15, (0x42 * 2));
  sensor_vl53lx_top = new VL53LX_X_NUCLEO_53L3A2(&DEV_I2C, xshutdown_top, A2);

  // Switch off VL53LX top component.
  sensor_vl53lx_top->VL53LX_Off();

#ifdef SATELLITES_MOUNTED
  // Create (if present) VL53LX left component.
  xshutdown_left = new STMPE1600DigiOut(&DEV_I2C, GPIO_14, (0x43 * 2));
  sensor_vl53lx_left = new VL53LX_X_NUCLEO_53L3A2(&DEV_I2C, xshutdown_left, D8);

  //Switch off (if present) VL53LX left component.
  sensor_vl53lx_left->VL53LX_Off();

  // Create (if present) VL53LX right component.
  xshutdown_right = new STMPE1600DigiOut(&DEV_I2C, GPIO_15, (0x43 * 2));
  sensor_vl53lx_right = new VL53LX_X_NUCLEO_53L3A2(&DEV_I2C, xshutdown_right, D2);

  // Switch off (if present) VL53LX right component.
  sensor_vl53lx_right->VL53LX_Off();
#endif

  //Initialize all the sensors
  sensor_vl53lx_top->InitSensor(0x10);
#ifdef SATELLITES_MOUNTED
  sensor_vl53lx_left->InitSensor(0x12);
  sensor_vl53lx_right->InitSensor(0x14);
#endif

  // Start Measurements
  sensor_vl53lx_top->VL53LX_StartMeasurement();
#ifdef SATELLITES_MOUNTED
  sensor_vl53lx_left->VL53LX_StartMeasurement();
  sensor_vl53lx_right->VL53LX_StartMeasurement();
#endif
}

void loop()
{
  VL53LX_MultiRangingData_t MultiRangingData;
  VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
  uint8_t NewDataReady = 0;
  int no_of_object_found = 0, j;
  char report[64];
  int status;

  do {
    status = sensor_vl53lx_top->VL53LX_GetMeasurementDataReady(&NewDataReady);
  } while (!NewDataReady);

  //Led on
  digitalWrite(13, HIGH);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_vl53lx_top->VL53LX_GetMultiRangingData(pMultiRangingData);
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    snprintf(report, sizeof(report), "VL53LX Top: Count=%d, #Objs=%1d ", pMultiRangingData->StreamCount, no_of_object_found);
    SerialPort.print(report);
    for (j = 0; j < no_of_object_found; j++) {
      if (j != 0) {
        SerialPort.print("\r\n                               ");
      }
      SerialPort.print("status=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeStatus);
      SerialPort.print(", D=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeMilliMeter);
      SerialPort.print("mm");
      SerialPort.print(", Signal=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].SignalRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps, Ambient=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].AmbientRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps");
    }
    SerialPort.println("");
    if (status == 0) {
      status = sensor_vl53lx_top->VL53LX_ClearInterruptAndStartMeasurement();
    }
  }

  digitalWrite(13, LOW);

#ifdef SATELLITES_MOUNTED

  NewDataReady = 0;
  no_of_object_found = 0;

  do {
    status = sensor_vl53lx_left->VL53LX_GetMeasurementDataReady(&NewDataReady);
  } while (!NewDataReady);

  //Led on
  digitalWrite(13, HIGH);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_vl53lx_left->VL53LX_GetMultiRangingData(pMultiRangingData);
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    snprintf(report, sizeof(report), "VL53LX Left: Count=%d, #Objs=%1d ", pMultiRangingData->StreamCount, no_of_object_found);
    SerialPort.print(report);
    for (j = 0; j < no_of_object_found; j++) {
      if (j != 0) {
        SerialPort.print("\r\n                                ");
      }
      SerialPort.print("status=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeStatus);
      SerialPort.print(", D=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeMilliMeter);
      SerialPort.print("mm");
      SerialPort.print(", Signal=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].SignalRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps, Ambient=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].AmbientRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps");
    }
    SerialPort.println("");
    if (status == 0) {
      status = sensor_vl53lx_left->VL53LX_ClearInterruptAndStartMeasurement();
    }
  }

  digitalWrite(13, LOW);

  NewDataReady = 0;
  no_of_object_found = 0;

  do {
    status = sensor_vl53lx_right->VL53LX_GetMeasurementDataReady(&NewDataReady);
  } while (!NewDataReady);

  //Led on
  digitalWrite(13, HIGH);

  if ((!status) && (NewDataReady != 0)) {
    status = sensor_vl53lx_right->VL53LX_GetMultiRangingData(pMultiRangingData);
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    snprintf(report, sizeof(report), "VL53LX Right: Count=%d, #Objs=%1d ", pMultiRangingData->StreamCount, no_of_object_found);
    SerialPort.print(report);
    for (j = 0; j < no_of_object_found; j++) {
      if (j != 0) {
        SerialPort.print("\r\n                                 ");
      }
      SerialPort.print("status=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeStatus);
      SerialPort.print(", D=");
      SerialPort.print(pMultiRangingData->RangeData[j].RangeMilliMeter);
      SerialPort.print("mm");
      SerialPort.print(", Signal=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].SignalRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps, Ambient=");
      SerialPort.print((float)pMultiRangingData->RangeData[j].AmbientRateRtnMegaCps / 65536.0);
      SerialPort.print(" Mcps");
    }
    SerialPort.println("");
    if (status == 0) {
      status = sensor_vl53lx_right->VL53LX_ClearInterruptAndStartMeasurement();
    }
  }

  digitalWrite(13, LOW);
#endif
}
