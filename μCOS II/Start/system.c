#include "system.h"

void SystemInit(void) {
    // 1. ����HSE��������·ģʽ
    RCC_CR |= (1 << 16);       // ����HSE
    RCC_CR &= ~(1 << 18);      // ������·ģʽ
    while (!(RCC_CR & (1 << 17))); // �ȴ�HSE����

    // 2. ����Flash�ȴ�����
    FLASH_ACR |= 0x02;         // 2�ȴ�����

    // 3. ����PLL����
    RCC_PLLCFGR = 0;
    RCC_PLLCFGR |= (8 << 0);      // PLL_M=8
    RCC_PLLCFGR |= (336 << 6);    // PLL_N=336
    RCC_PLLCFGR &= ~(0x3 << 16);  // ���PLLP��ֵ
    RCC_PLLCFGR |= (0x1 << 16);   // PLL_P=4��Ƶ
    RCC_PLLCFGR |= (1 << 22);     // PLLԴѡ��HSE

    // 4. ����PLL���ȴ�����
    RCC_CR |= (1 << 24);       // ����PLL
    while (!(RCC_CR & (1 << 25))); // �ȴ�PLL����

    // 5. �������߷�Ƶ
    RCC_CFGR &= ~(0xF << 4);   // AHB��Ƶ=1��84MHz��
    RCC_CFGR |= (4 << 10);      // APB1��Ƶ=2��42MHz��
    RCC_CFGR &= ~(0x7 << 13);   // APB2��Ƶ=1��84MHz��

    // 6. �л�ϵͳʱ�ӵ�PLL
    RCC_CFGR &= ~0x03;         // ���SWλ
    RCC_CFGR |= 0x02;          // SW=0b10��PLL��Ϊϵͳʱ�ӣ�
    while ((RCC_CFGR & 0x0C) != 0x08); // �ȴ��л����
}
