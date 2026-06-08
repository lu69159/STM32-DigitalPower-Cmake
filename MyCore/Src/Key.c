#include "Key.h"

extern uint8_t OUT_Status;
extern uint8_t OC_Status;

typedef enum
{
	KS_RELEASE = 0,
	KS_SHAKE,
	KS_PRESS,
} KEY_STATUS;

#define g_keyStatus 0
#define g_nowKeyStatus 1
#define g_lastKeyStatus 2

uint8_t KEY_Status[KEY_NUM + 1][3];
uint8_t Key_Flag[KEY_NUM + 1];
uint8_t Key_Flag_LOCK[KEY_NUM + 1];

void Key_Init(void)
{
	uint8_t i;
	for (i = 0; i < KEY_NUM + 1; i++)
	{
		KEY_Status[i][g_keyStatus] = KS_RELEASE;
		KEY_Status[i][g_nowKeyStatus] = KS_RELEASE;
		KEY_Status[i][g_lastKeyStatus] = KS_RELEASE;
		Key_Flag[i] = 0;
		Key_Flag_LOCK[i] = 0;
	}
}

void KEY_Scan(uint8_t key_num, uint8_t KEY)
{
	switch (KEY_Status[key_num][g_keyStatus])
	{
	case KS_RELEASE:
	{
		if (KEY == 0)
		{
			KEY_Status[key_num][g_keyStatus] = KS_SHAKE;
		}
	}
	break;

	case KS_SHAKE:
	{
		if (KEY == 1)
		{
			KEY_Status[key_num][g_keyStatus] = KS_RELEASE;
		}
		else
		{
			KEY_Status[key_num][g_keyStatus] = KS_PRESS;
		}
	}
	break;

	case KS_PRESS:
	{
		if (KEY == 1)
		{
			KEY_Status[key_num][g_keyStatus] = KS_SHAKE;
		}
		else
		{
			if (Key_Flag_LOCK[key_num] == 0)
			{
				Key_Flag[key_num] = 1;
				Key_Flag_LOCK[key_num] = 1;
			}
		}
	}
	break;

	default:
		break;
	}

	if (KEY_Status[key_num][g_keyStatus] != KEY_Status[key_num][g_nowKeyStatus])
	{
		if ((KEY_Status[key_num][g_keyStatus] == KS_RELEASE) && (KEY_Status[key_num][g_lastKeyStatus] == KS_PRESS))
		{
			Key_Flag_LOCK[key_num] = 0;
		}
		KEY_Status[key_num][g_lastKeyStatus] = KEY_Status[key_num][g_nowKeyStatus];
		KEY_Status[key_num][g_nowKeyStatus] = KEY_Status[key_num][g_keyStatus];
	}
}
