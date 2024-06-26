#include <library.h>
#include "../../os_host/source/framework/Console.h"
#include "../../os_host/source/framework/SimpleApp.h"
#include "../../os_host/source/framework/BufferedIo.h"
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
//#include "../../os_host/library/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c"
//#include "../../os_host/library/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c"
//#undef FLAG_Mask
//#include "../../os_host/library/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c"

#include "gpio.h"
#include <math.h>

volatile bool flag1 = false;
volatile bool flag2 = false;

// dma table: https://deepbluembedded.com/stm32-dma-tutorial-using-direct-memory-access-dma-in-stm32/

#define __irq__ __attribute__((interrupt("IRQ"), optimize("O2")))

void crash_with_message(const char *message, void *caller)
{
  BIOS::DBG::Print("Error: %s", message);
  while (1);
}

void __irq__ DMA1_Channel4_IRQHandler()
{
    if (DMA1->ISR & DMA_ISR_TEIF4)
    {
        crash_with_message("Oh noes: DMA channel 4 transfer error!",
            __builtin_return_address(0)
        );
        while(1);
    }
    else if (DMA1->ISR & DMA_ISR_HTIF4)
    {
//        process_samples(&adc_fifo[0]);
        flag1 = true;
        DMA1->IFCR = DMA_IFCR_CHTIF4;
        if (DMA1->ISR & DMA_ISR_TCIF4)
        {
            crash_with_message("Oh noes: ADC fifo overflow in HTIF", __builtin_return_address(0));
            while(1);
        }
    }
    else if (DMA1->ISR & DMA_ISR_TCIF4)
    {
//        process_samples(&adc_fifo[ADC_FIFO_HALFSIZE]);
        flag2 = true;
        DMA1->IFCR = DMA_IFCR_CTCIF4;
        if (DMA1->ISR & DMA_ISR_HTIF4)
        {
            crash_with_message("Oh noes: ADC fifo overflow in TCIF", __builtin_return_address(0));
            while(1);
        }
    }
}


/*
  PB12: WS                I2S2_WS   P4     LRCK             LRC
  PB13: SCK   SPI2_SCK    I2S2_CK   P3     BICK             BCLK
  PB14:       SPI2_MISO             P2
  PB15: SD    SPI2_MOSI   I2S2_SD   P1     SDTI data?       DIK

  PC6 I2S2_MCK
*/

uint16_t buffer[4096];
uint16_t pix[200];

__attribute__((__section__(".entry")))
int main(void)
{
    APP::Init("I2S Test1");

    Gpio::SetState(Gpio::BASEB, Gpio::P1, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P2, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P3, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P4, Gpio::StateInput | Gpio::StateInputFloating);

    Gpio::SetState(Gpio::BASEB, Gpio::P2MISO, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P3SCK2, Gpio::StateOutput10Mhz | Gpio::StateOutputFunctionPushPull);
    Gpio::SetState(Gpio::BASEB, Gpio::P4CS, Gpio::StateOutput10Mhz | Gpio::StateOutputFunctionPushPull);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    RCC->APB1ENR |= RCC_APB1Periph_SPI2;
    RCC->APB2ENR |= RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;

    I2S_InitTypeDef initStruct;
    I2S_StructInit(&initStruct);

    initStruct.I2S_Mode = I2S_Mode_MasterRx;
    initStruct.I2S_Standard = I2S_Standard_Phillips;
    initStruct.I2S_DataFormat = I2S_DataFormat_24b;
    initStruct.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    initStruct.I2S_AudioFreq = I2S_AudioFreq_44k; // 8, 11, 16, 22, 32, 44, 48, 96, 192
    initStruct.I2S_CPOL = I2S_CPOL_Low;

    BIOS::OS::SetInterruptVector(BIOS::OS::EInterruptVector::IDMA1_Channel4_IRQ, DMA1_Channel4_IRQHandler);

    I2S_Init(SPI2, &initStruct);
    I2S_Cmd(SPI2, ENABLE);

    for (int i=0; i<COUNT(buffer); i+=1)
      buffer[i] = 3;

    DMA_InitTypeDef DMA_InitStructure;
        DMA_DeInit(DMA1_Channel4);
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uintptr_t)&SPI2->DR;
        DMA_InitStructure.DMA_MemoryBaseAddr = (uintptr_t)buffer;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStructure.DMA_BufferSize = COUNT(buffer);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        DMA_Init(DMA1_Channel4, &DMA_InitStructure);

        DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
        DMA_ITConfig(DMA1_Channel4, DMA_IT_TE, ENABLE);
        DMA_ITConfig(DMA1_Channel4, DMA_IT_HT, ENABLE);
        
        /* Disable SPI3 DMA TX request */
        SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
        DMA_Cmd (DMA1_Channel4, DISABLE);

    // Clear any pending interrupts for ch 4
    DMA1->IFCR = 0x0000F000;
    
    // Enable ch 5 interrupt
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    NVIC_SetPriority(DMA1_Channel4_IRQn, 0); // Highest priority

        DMA_Cmd (DMA1_Channel4, ENABLE);

//    int16_t sample;
    for (int i=0; i<200; i++)
      pix[i] = RGB565(404080);
    (void)pix[7];

    BIOS::SYS::DelayMs(30);
    int ofs = 0;
    if (buffer[0] == 0 && buffer[3] == 0)
      ofs = 1;
    else if (buffer[0] == 0 && buffer[1] == 0)
      ofs = 2;
    BIOS::LCD::BufferBegin(CRect(0, 16, BIOS::LCD::Width, 16+200));
    while (!BIOS::KEY::GetKey())
    {
//      BIOS::SYS::DelayMs(30);
//if (0)
      int index = ofs;
      for (int i=0; i<320; i++)
      {
          int32_t vv1 = (buffer[index]<<4)|(buffer[index+1]>>12);
          index += 4;
          if (vv1 & 0x80000)
            vv1 |= ~0x7ffff;
          int y1 = vv1/200 + 200;
          if (y1 < 0) y1 = 0;
          if (y1 > 199) y1 = 199;  

          int32_t vv2 = (buffer[index]<<4)|(buffer[index+1]>>12);
          index += 4;
          if (vv2 & 0x80000)
            vv2 |= ~0x7ffff;
          int y2 = vv2/200 + 200;
          if (y2 < 0) y2 = 0;
          if (y2 > 199) y2 = 199;  

          pix[y1] = RGB565(ffffff);
          pix[y2] = RGB565(ffffff);
          BIOS::LCD::BufferWrite(pix, 200);
          pix[y1] = RGB565(404080);
          pix[y2] = RGB565(404080);
      }
//      BIOS::LCD::Printf(20, 200, RGB565(ffffff), RGB565(404080), "A %x %x %x %x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
//      BIOS::LCD::Printf(20, 216, RGB565(ffffff), RGB565(404080), "B %x %x %x %x\n", buffer[4], buffer[5], buffer[6], buffer[7]);

    }
    BIOS::LCD::BufferEnd();
    DMA_Cmd (DMA1_Channel4, DISABLE);
    I2S_Cmd(SPI2, DISABLE);
    Gpio::SetState(Gpio::BASEB, Gpio::P1MOSI, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P2MISO, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P3SCK2, Gpio::StateInput | Gpio::StateInputFloating);
    Gpio::SetState(Gpio::BASEB, Gpio::P4CS, Gpio::StateInput | Gpio::StateInputFloating);

    // warning: where does it point?
    BIOS::OS::SetInterruptVector(BIOS::OS::EInterruptVector::IDMA1_Channel5_IRQ, [](){});

    return 0;
}

void _HandleAssertion(const char* file, int line, const char* cond)
{
    BIOS::DBG::Print("Assertion failed in %s [%d]: %s\n", file, line, cond);
    while (1);
}
