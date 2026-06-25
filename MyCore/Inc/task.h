/*
 * task.h
 *
 *  Created on: Jun 25, 2026
 *      Author: Administrator1
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

#include "stm32g474xx.h"

enum pageState{ //页面状态
    pageMenu,
	pageChoose,
    pageEdit
};

enum keyState{ //按键状态
    keyIdle,
    keyShake,
    keyPressed
};

typedef struct{
    GPIO_TypeDef *Port;
    uint16_t Pin;
    enum keyState state;
    int isLocked;
}Key;

void taskInit();
void taskRun();

//OLED
void oledPlay();

//KEY
void key(Key *key);
void keyListening(Key *key);

//ADC
void getADCresult();

#endif /* INC_TASK_H_ */
