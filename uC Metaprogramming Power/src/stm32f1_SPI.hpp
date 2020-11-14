#ifndef _STM32F1_SPI_HPP
#define _STM32F1_SPI_HPP

#include "stm32f1_Power.hpp"

namespace controller{

template<auto baseAddress>
class SPI{

  static const uint32_t RCC_AHBENR_DMA1EN = 1;
  static const uint32_t RCC_APB2ENR_IOPBEN = 8;
  static const uint32_t RCC_APB1ENR_SPI2EN = 0x4000;

  /*!
    @brief Trait for using in Power class. Consists of Valueslist with
      values for AHBENR, APB1ENR, APB2ENR registers 
  */
  using power = Power::fromValues<
                                  RCC_AHBENR_DMA1EN,
                                  RCC_APB1ENR_SPI2EN, 
                                  RCC_APB2ENR_IOPBEN>::power;
                                  
  template<typename>
  friend class interfaces::IPower;
};

}

#endif // !_STM32F1_SPI_HPP