#ifndef _STM32F1_UART_HPP
#define _STM32F1_UART_HPP

#include "stm32f1_Power.hpp"

namespace controller{

template<auto baseAddress>
class UART{

  static const uint32_t RCC_AHBENR_DMA1EN = 1;
  static const uint32_t RCC_APB2ENR_IOPAEN = 4;
  static const uint32_t RCC_APB2ENR_USART1EN = 0x4000;

  /*!
    @brief Trait for using in Power class. Consists of Valueslist with
      values for AHBENR, APB1ENR, APB2ENR registers 
  */
  using power = Power::fromValues<
                                  RCC_AHBENR_DMA1EN,
                                  0U, 
                                  RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN>::power;
                                  
  template<typename>
  friend class interfaces::IPower;
};

}

#endif // !_STM32F1_UART_HPP