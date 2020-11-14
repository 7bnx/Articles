#ifndef _IPOWER_HPP
#define _IPOWER_HPP

#include "type_traits_custom.hpp"

#define __FORCE_INLINE __attribute__((always_inline)) inline


/*!
  @brief Controller's peripherals interfaces
*/
namespace controller::interfaces{

/*!
  @brief Interface for Power(Clock control). Static class. CRT pattern
  @tparam <adapter> class of specific controller
*/
template<typename adapter>  
class IPower{

  IPower() = delete;

public:

  /*!
    @brief Enables peripherals Power(Clock)
    @tparam <Peripherals> list of peripherals with trait 'power'
  */
  template<typename... Peripherals>
  __FORCE_INLINE static void Enable(){
    using tEnableList = utils::lists_termwise_or_t<typename Peripherals::power...>;
    using tDisableList = typename adapter::template fromValues<>::power;
   adapter:: template _Set<tEnableList, tDisableList>();
  }

  /*!
    @brief Enables Power(Clock) except listed peripherals in 'ExceptList'. 
      If Enable = Exception = 1, then Enable = 0, otherwise depends on Enable.
    @tparam <EnableList> list to enable, with trait 'power'
    @tparam <ExceptList> list of exception, with trait 'power'
  */
  template<typename EnableList, typename ExceptList>
  __FORCE_INLINE static void EnableExcept(){
    using tXORedList = utils::lists_termwise_xor_t<typename EnableList::power, typename ExceptList::power>;
    using tEnableList = utils::lists_termwise_and_t<typename EnableList::power, tXORedList>;
    using tDisableList = typename adapter::template fromValues<>::power;
    adapter:: template _Set<tEnableList, tDisableList>();
  }

  /*!
    @brief Disables peripherals Power(Clock)
    @tparam <Peripherals> list of peripherals with trait 'power'
  */
  template<typename... Peripherals>
  __FORCE_INLINE static void Disable(){
    using tDisableList = utils::lists_termwise_or_t<typename Peripherals::power...>;
    using tEnableList = typename adapter::template fromValues<>::power;
    adapter:: template _Set<tEnableList, tDisableList>();
  }

  /*!
    @brief Disables Power(Clock) except listed peripherals in 'ExceptList'. 
      If Disable = Exception = 1, then Disable = 0, otherwise depends on Disable.
    @tparam <DisableList> list to disable, with trait 'power'
    @tparam <ExceptList> list of exception, with trait 'power'
  */
  template<typename DisableList, typename ExceptList>
  __FORCE_INLINE static void DisableExcept(){
    using tXORedList = utils::lists_termwise_xor_t<typename DisableList::power, typename ExceptList::power>;
    using tDisableList = utils::lists_termwise_and_t<typename DisableList::power, tXORedList>;
    using tEnableList = typename adapter::template fromValues<>::power;
    adapter:: template _Set<tEnableList, tDisableList>();
  }

  /*!
    @brief Disable and Enables Power(Clock) depends on values. 
      If Enable = Disable = 1, then Enable = Disable = 0, otherwise depends on values
    @tparam <EnableList> list to enable, with trait 'power'
    @tparam <DisableList> list to disable, with trait 'power'
  */
  template<typename EnableList, typename DisableList>
  __FORCE_INLINE static void Keep(){
    using tXORedList = utils::lists_termwise_xor_t<typename EnableList::power, typename DisableList::power>;
    using tEnableList = utils::lists_termwise_and_t<typename EnableList::power, tXORedList>;
    using tDisableList = utils::lists_termwise_and_t<typename DisableList::power, tXORedList>;
    adapter:: template _Set<tEnableList, tDisableList>();
  }

  /*!
    @brief Creates custom 'power' list from peripherals. Peripheral driver should implement 'power' trait.
      E.g.: using power = Power::makeFromValues<1, 512, 8>::power; 
    @tparam <PeripheralsList> list of peripherals with trait 'power'
  */
 template<typename... PeripheralsList>
  class fromPeripherals{
    fromPeripherals() = delete;
    using power = utils::lists_termwise_or_t<typename PeripheralsList::power...>;
    friend class IPower<adapter>;
  };

};

} // !namespace controller::interfaces


#undef   __FORCE_INLINE

#endif // !_IPOWER_HPP