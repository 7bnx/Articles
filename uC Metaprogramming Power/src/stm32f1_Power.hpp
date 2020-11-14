#ifndef _STM32F1_POWER_HPP
#define _STM32F1_POWER_HPP

#include <cstdint>
#include "IPower.hpp"
#include "HPower.hpp"
#include "type_traits_custom.hpp"

#define __FORCE_INLINE __attribute__((always_inline)) inline

/*!
  @brief Controller's peripherals
*/
namespace controller{

/*!
  @brief Power managment for controller
*/
class Power: public interfaces::IPower<Power>, public hardware::HPower{

  Power() = delete;

public:

  /*!
    @brief Creates custom 'power' list from values. Peripheral driver should implement 'power' trait.
      E.g.: using power = Power::fromValues<1, 512, 8>::power; 
    @tparam <valueAHB=0> value for AHBENR register
    @tparam <valueAPB1=0> value for APB1ENR register
    @tparam <valueAPB2=0> value for APB1ENR register
  */
  template<uint32_t valueAHBENR = 0, uint32_t valueAPB1ENR = 0, uint32_t valueAPB2ENR = 0>
  struct fromValues{
    fromValues() = delete;
    using power = utils::Valuelist<valueAHBENR, valueAPB1ENR, valueAPB2ENR>;
  };

private: 

  static constexpr uint32_t 
    _addressAHBENR  = 0x40021014,
    _addressAPB2ENR = 0x40021018,
    _addressAPB1ENR = 0x4002101C;
  
  using AddressesList = utils::Valuelist<_addressAHBENR, _addressAPB1ENR, _addressAPB2ENR>;

  template<typename EnableList, typename DisableList>
  __FORCE_INLINE static void _Set(){
    HPower:: template ModifyRegisters<EnableList, DisableList, AddressesList>();
  }

  friend class IPower<Power>;

};

} // !namespace controller

#undef __FORCE_INLINE

#endif // !_STM32F1_POWER_HPP