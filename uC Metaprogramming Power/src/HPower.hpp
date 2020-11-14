#ifndef _HPOWER_HPP
#define _HPOWER_HPP

#include "type_traits_custom.hpp"

#define __FORCE_INLINE __attribute__((always_inline)) inline

/*!
  @brief Hardware operations
*/
namespace controller::hardware{

/*!
  @brief Implements hardware operations with Power(Clock) registers
*/
class HPower{

protected:

  HPower() = delete;

/*!
  @brief Set or Reset bits in the registers
  @tparam <SetList> list of values to set 
  @tparam <ResetList> list of values to reset
  @tparam <AddressesList> list of registers addresses to operate
*/
  template<typename SetList, typename ResetList, typename AddressesList>
  __FORCE_INLINE static void ModifyRegisters(){
    using namespace utils;

    if constexpr (!is_empty_v<SetList> && !is_empty_v<ResetList> && !is_empty_v<AddressesList>){

      constexpr auto valueSet = front_v<SetList>;
      constexpr auto valueReset = front_v<ResetList>;

      if constexpr(valueSet || valueReset){
        constexpr auto address = front_v<AddressesList>;
          
        using pRegister_t = volatile std::remove_const_t<decltype(address)>* const;
        auto& reg = *reinterpret_cast<pRegister_t>(address);

        reg = (reg &(~valueReset)) | valueSet;
      }
        
      using tRestSet = pop_front_t<SetList>;
      using tRestReset = pop_front_t<ResetList>;
      using tRestAddress = pop_front_t<AddressesList>;
      
      ModifyRegisters<tRestSet, tRestReset, tRestAddress>();
    }
  };

};

} // !namespace controller::hardware

#undef __FORCE_INLINE

#endif // !_HPOWER_HPP