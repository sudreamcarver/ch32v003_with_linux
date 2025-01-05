#include <debug.h>

void GPIO_Toggle(void) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

int main(void) {
  u8 i = 0;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  SystemCoreClockUpdate();
  Delay_Init();
  printf("SystemClk:%d\r\n", SystemCoreClock);
  printf("chipid:%08x\r\n", DBGMCU_GetDEVID());
  printf("GPIO Toggle TEST\r\n");

  GPIO_Toggle();
  while (1) {
    Delay_Ms(250);
    GPIO_WriteBit(GPIOD, GPIO_Pin_0,
                  (i == 0) ? (i == Bit_SET) : (i = Bit_RESET));
  }
}
