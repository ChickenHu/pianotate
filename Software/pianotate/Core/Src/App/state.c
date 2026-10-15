#include "state.h"
#include "key.h"
#include "led.h"
#include "mpu6050.h"
#include "fourkeys.h"
#include "menu.h"
#include "oled.h"
#include "piano.h"
#include "stm32f1xx_hal_gpio.h"
#include "timer.h"
#include <stdint.h>

void State_Update() {
  static bool mpuTickInitialized = false;
  static uint32_t lastMpuTick;
  static uint32_t lastMenuUpdate = 0;
  static uint32_t lastFrameUpdate = 0;
  static bool gameFramePending = false;
  static bool refreshOLED1Next = true;

  if (!mpuTickInitialized) {
    lastMpuTick = Timer;
    mpuTickInitialized = true;
  }
  uint32_t mpuNow = Timer;
  uint32_t sampleElapsed = mpuNow - lastMpuTick;
  if (MPU6050_Ready && sampleElapsed >= 10u) {
    lastMpuTick = mpuNow;
    if (MPU6050_Read(&MPU6050_Device) == HAL_OK) {
      uint16_t elapsedMs = sampleElapsed > 0xFFFFu
                               ? 0xFFFFu
                               : (uint16_t)sampleElapsed;
      MPU6050_UpdateFlags(&MPU6050_Device, elapsedMs);
    }
  }

  // 所有状态机的状态更新都放在此处，统一维护
  Key_Update();
  Piano_RecordUpdate();
  FourKeys_JudgeInput();
  for (uint8_t i = 0; i < 7; i++) {
    if (KeyList[i].state == KEY_PRESSED) {
      HAL_GPIO_WritePin(LED[i].gpio, LED[i].pin, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(LED[i].gpio, LED[i].pin, GPIO_PIN_RESET);
    }
  }
  Music_UpdateState();

  uint32_t now = Timer;
  if ((uint32_t)(now - lastMenuUpdate) >= FreshPeriod) {
    lastMenuUpdate = now;
    Menu_StateUpdate();
  }

  if (!gameFramePending &&
      (uint32_t)(now - lastFrameUpdate) >= (uint32_t)FreshPeriod * 2u) {
    lastFrameUpdate = now;
    bool fourKeysWasEnabled = StartFlag;
    Piano_Update();
    FourKeys_UpdateState();
    gameFramePending =
        fourKeysWasEnabled && OLED_PendingChanges(&OLED1);
  }
  bool oled1Pending = OLED_PendingChanges(&OLED1);
  bool oled2Pending = OLED_PendingChanges(&OLED2);
  if (oled1Pending && oled2Pending) {
    OLED_HandleTypeDef *oled = refreshOLED1Next ? &OLED1 : &OLED2;
    OLED_UpdateState(oled);
    refreshOLED1Next = !refreshOLED1Next;
  } else if (oled1Pending) {
    OLED_UpdateState(&OLED1);
    refreshOLED1Next = false;
  } else if (oled2Pending) {
    OLED_UpdateState(&OLED2);
    refreshOLED1Next = true;
  }
  if (gameFramePending && !OLED_PendingChanges(&OLED1)) {
    gameFramePending = false;
  }
}