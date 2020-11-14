#ifndef _TYPE_TRAITS_CUSTOM_HPP
#define _TYPE_TRAITS_CUSTOM_HPP

#include <type_traits>

/*!
  @file
  @brief Traits for metaprogramming
*/

/*!
  @brief Namespace for utils.
*/
namespace utils{

/*-----------------------------------Basic----------------------------------------*/

/*!
  @brief Basic list of types
  @tparam Types parameter pack
*/
template<typename... Types>
struct Typelist{};

/*!
  @brief Basic list of values
  @tparam Values parameter pack
*/
template<auto... Values>
struct Valuelist{};

/*------------------------------End of Basic--------------------------------------*/


/*----------------------------------Front-------------------------------------------
  Description:  Pop front type or value from list

  using listOfTypes = Typelist<int, short, bool, unsigned>;
  using listOfValues = Valuelist<1,2,3,4,5,6,1>;

  |-----------------|--------------------|----------|
  |      Trait      |    Parameters      |  Result  |
  |-----------------|--------------------|----------|
  |     front_t     |   <listOfTypes>    |    int   |
  |-----------------|--------------------|----------|
  |     front_v     |   <listOfValues>   |     1    |
  |-----------------|--------------------|----------| */

namespace{

template<typename List>
struct front;

template<typename Head, typename... Tail>
struct front<Typelist<Head, Tail...>>{ 
  using type = Head; 
};

template<auto Head, auto... Tail>
struct front<Valuelist<Head, Tail...>> {
  static constexpr auto value = Head;
};

}

template<typename List>
using front_t = typename front<List>::type;

template<typename List>
static constexpr auto front_v = front<List>::value;

/*----------------------------------End of Front----------------------------------*/


/*----------------------------------Pop_Front---------------------------------------
  Description:  Pop front type or value from list and return rest of the list

  using listOfTypes = Typelist<int, short, bool>;
  using listOfValues = Valuelist<1,2,3,4,5,6,1>;

  |-----------------|--------------------|------------------------|
  |      Trait      |    Parameters      |         Result         |
  |-----------------|--------------------|------------------------|
  |   pop_front_t   |    <listOfTypes>   | Typelist<short, bool>  |
  |-----------------|--------------------|------------------------|
  |   pop_front_t   |   <listOfValues>   | Valuelist<2,3,4,5,6,1> |
  |-----------------|--------------------|------------------------| */

namespace{

template<typename List>
struct pop_front;

template<typename Head, typename... Tail>
struct pop_front<Typelist<Head, Tail...>> {
  using type = Typelist<Tail...>;
};

template<auto Head, auto... Tail>
struct pop_front<Valuelist<Head, Tail...>> {
  using type = Valuelist<Tail...>;
};

}

template<typename List>
using pop_front_t = typename pop_front<List>::type;

/*------------------------------End of Pop_Front----------------------------------*/


/*----------------------------------Push_Front--------------------------------------
  Description:  Push new element to front of the list

  using listOfTypes = Typelist<short, bool>;

  |-----------------------|--------------------------|-------------------------------|
  |      Trait            |        Parameters        |             Result            |
  |-----------------------|--------------------------|-------------------------------|
  |      push_front_t     |   <listOfTypes, float>   | Typelist<float, short, bool>  |
  |-----------------------|--------------------------|-------------------------------| */

namespace{

template<typename List, typename NewElement>
struct push_front;

template<typename... List, typename NewElement>
struct push_front<Typelist<List...>, NewElement> {
  using type = Typelist<NewElement, List...>;
};

}

template<typename List, typename NewElement>
using push_front_t = typename push_front<List, NewElement>::type;


/*------------------------------End of Push_Front---------------------------------*/


/*----------------------------------Push_Back---------------------------------------
  Description:  Push new value to back of the list

  using listOfValues = Valuelist<1,2,3,4,5,6>;

  |-----------------------|--------------------------|-------------------------------|
  |      Trait            |        Parameters        |             Result            |
  |-----------------------|--------------------------|-------------------------------|
  |   push_back_value_t   |     <listOfValues, 0>    |    Valuelist<1,2,3,4,5,6,0>   |
  |-----------------------|--------------------------|-------------------------------| */

namespace{

template<typename List, auto NewElement>
struct push_back_value;

template<auto... List, auto NewElement>
struct push_back_value<Valuelist<List...>, NewElement>{
  using type = Valuelist<List..., NewElement>;
};

}

template<typename List, auto NewElement>
using push_back_value_t = typename push_back_value<List, NewElement>::type;

/*----------------------------------End of Push_Back------------------------------*/

/*-----------------------------------Is_Empty---------------------------------------
  Description:  Check parameters list for empty and return bool value

  using listOfTypes = Typelist<int, short, bool, unsigned>;
  using listOfValues = Valuelist<>;

  |-------------------------|--------------------|----------|
  |          Trait          |     Parameters     |  Result  |
  |-------------------------|--------------------|----------|
  |        is_empty_v       |    <listOfTypes>   |  false   |
  |-------------------------|--------------------|----------|
  |        is_empty_v       |   <listOfValues>   |   true   |
  |-------------------------|--------------------|----------| */

namespace{
/*!
  @brief Check the emptiness of the types in parameters.   \n 
    E.g.: is_empty<int, short, bool>::value;
*/ 
template<typename List>
struct is_empty{
    static constexpr auto value = false;
};

/*!
  @brief Check the emptiness of the types in parameter. Specializatio for empty parameters   \n 
    E.g.: is_empty<>::value;
*/ 
template<>
struct is_empty<Typelist<>>{
    static constexpr auto value = true;
};

template<>
struct is_empty<Valuelist<>>{
    static constexpr auto value = true;
};

}

/*!
  @brief Check the emptiness of the types-list in parameter.   \n 
    E.g.: using list = Typelist<int, short, bool>; is_empty_v<list>;
*/ 
template<typename List>
static constexpr auto is_empty_v = is_empty<List>::value;

/*--------------------------------End of Is_Empty---------------------------------*/


/*---------------------------------Size_Of_List-------------------------------------
  Description:  Return number of elements in list

  using listOfTypes = Typelist<int, float, double, bool>;

  |------------------|--------------------|----------|
  |       Trait      |     Parameters     |  Result  |
  |------------------|--------------------|----------|
  |  size_of_list_v  |     listOfTypes    |    4     |
  |------------------|--------------------|----------| */

namespace{

template<typename List, std::size_t count = 0U>
struct size_of_list : public size_of_list<pop_front_t<List>, count + 1>{};

template<std::size_t count>
struct size_of_list<Typelist<>, count>{
  static constexpr std::size_t value = count;
};

template<std::size_t count>
struct size_of_list<Valuelist<>, count>{
  static constexpr std::size_t value = count;
};

}

template<typename List>
static constexpr std::size_t size_of_list_v = size_of_list<List>::value;

/*-------------------------------End Size_Of_List---------------------------------*/

/*---------------------------------Lists Operation--------------------------------*/

  /*Description: Operations with lists of values

  using list1 = Valuelist<1, 4, 8, 16>;
  using list2 = Valuelist<1, 5, 96, 17>;

  |------------------------------|-------------------|---------------------------|
  |               Trait          |    Parameters     |           Result          |
  |------------------------------|-------------------|---------------------------|
  |     lists_termwise_and_t     |  <list1, list2>   |  Valuelist<1, 4, 0, 16>   |
  |------------------------------|-------------------|---------------------------|
  |     lists_termwise_or_t      |  <list1, list2>   |  Valuelist<1, 5, 104, 17> |
  |---------------------------- -|-------------------|---------------------------|
  |     lists_termwise_xor_t     |  <list1, list2>   |  Valuelist<0, 1, 104, 1>  |
  |------------------------------|-------------------|---------------------------| */

namespace{

template<template <auto value1, auto value2> typename operation, 
         typename List1, typename List2, typename Result = Valuelist<>>
struct operation_2_termwise_valuelists{
  constexpr static auto newValue = operation<front_v<List1>, front_v<List2>>::value;
  using nextList1 = pop_front_t<List1>;
  using nextList2 = pop_front_t<List2>;
    
  using result = push_back_value_t<Result, newValue>;
  using type = typename 
      operation_2_termwise_valuelists<operation, nextList1, nextList2, result>::type;
};

template<template <auto value1, auto value2> typename operation, typename Result>
struct operation_2_termwise_valuelists<operation, Valuelist<>, Valuelist<>, Result>{
  using type = Result;
};

template<template <auto value1, auto value2> typename operation, 
         typename List2, typename Result>
struct operation_2_termwise_valuelists<operation, Valuelist<>, List2, Result>{
  using type = typename 
      operation_2_termwise_valuelists<operation, Valuelist<0>, List2, Result>::type;
};

template<template <auto value1, auto value2> typename operation, 
         typename List1, typename Result>
struct operation_2_termwise_valuelists<operation, List1, Valuelist<>, Result>{
  using type = typename 
      operation_2_termwise_valuelists<operation, List1, Valuelist<0>, Result>::type;
};


template<template<typename first, typename second> class operation,
         typename Lists, bool isEnd = size_of_list_v<Lists> == 1>
class lists_operation{

  using first = front_t<Lists>;
  using second = front_t<pop_front_t<Lists>>;
  using next = pop_front_t<pop_front_t<Lists>>;
  using result = operation<first, second>;

public:

  using type = typename lists_operation<operation, push_front_t<next, result>>::type;

};

template<template<typename first, typename second> class operation,
         typename Lists>
class lists_operation<operation, Lists, true>{
public:
  using type = front_t<Lists>;
};


template<auto value1, auto value2>
struct and_operation{ static constexpr auto value = value1 & value2;};

template<auto value1, auto value2>
struct or_operation{ static constexpr auto value = value1 | value2;};

template<auto value1, auto value2>
struct xor_operation{ static constexpr auto value = value1 ^ value2;};


template<typename List1, typename List2>
using operation_and_termwise_t = typename 
    operation_2_termwise_valuelists<and_operation, List1, List2>::type;

template<typename List1, typename List2>
using operation_or_termwise_t = typename 
    operation_2_termwise_valuelists<or_operation, List1, List2>::type;

template<typename List1, typename List2>
using operation_xor_termwise_t = typename 
    operation_2_termwise_valuelists<xor_operation, List1, List2>::type;

}

template<typename... Lists>
using lists_termwise_and_t = 
    typename lists_operation<operation_and_termwise_t, Typelist<Lists...>>::type;

template<typename... Lists>
using lists_termwise_or_t = typename 
    lists_operation<operation_or_termwise_t, Typelist<Lists...>>::type;

template<typename... Lists>
using lists_termwise_xor_t = typename 
    lists_operation<operation_xor_termwise_t, Typelist<Lists...>>::type;

/*--------------------------------End of Lists Operation----------------------------*/

} // !namespace utils

#endif //!_TYPE_TRAITS_CUSTOM_HPP