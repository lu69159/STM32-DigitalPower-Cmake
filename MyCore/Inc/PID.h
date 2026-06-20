#ifndef __PID_H
#define __PID_H

#include "main.h"

#define PERIOD 30000

void PID_Init(void);
void BuckBoostVILoopCtlPID(void);
void BuckBoostVILoopCtlPID_TEST(void);

#endif
