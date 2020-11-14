#include "stm32f1_Power.hpp"
#include "stm32f1_UART.hpp"
#include "stm32f1_SPI.hpp"

using namespace controller;

using spi = SPI<2>;
using uart = UART<1>;

using listPowerInit = Power::fromPeripherals<spi, uart>;
using listPowerDown = Power::fromPeripherals<spi>;
using listPowerWake = Power::fromPeripherals<uart>;

int main(){

  Power::Enable<listPowerInit>();

  //Some code

  Power::DisableExcept<listPowerDown, listPowerWake>();

  //Sleep();

  Power::EnableExcept<listPowerDown, listPowerWake>();

  while(1);
  return 1;
};