/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ConceptWrapper.h
 *  @brief A template wrapper that will either enforce functionality or provide default functions.
 *
 *  Starting in future versions of C++, a concept is a set of requirements for a class to be used
 *  in a template.  This wrapper around a class can either REQUIRE functions to be present in the
 *  internal class, or provide DEFAULT functionality when functions are missing.
 * 
 *  Use the EMP_BUILD_CONCEPT macro to create a new concept wrapper.  Provide it with the wrapper
 *  name, and all of the rules.  The allowable rule types are:
 *  
 *  REQUIRED_FUN ( FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG_TYPES... )
 *    Setup a function that is required to be present in the wrapped class.  If it does not
 *    exist there, throw the provided error.
 * 
 *  OPTIONAL_FUN ( FUNCTION_NAME, DEFAULT_ACTION, RETURN_TYPE, ARG_TYPES... )
 *    Setup a function.  If it exists, call the version in the wrapped class.  If it does not
 *    exist, return the default instead.  The function signature is needed as part of the 
 *    automated testing as to whether the function exists.
 * 
 *  PRIVATE ( CODE )
 *    All code provided will appear in the private portion of the wrapper.
 * 
 *  PROTECTED ( CODE )
 *    All code provided will appear in the protected portion of the wrapper.
 *  
 *  PUBLIC ( CODE )
 *    All code provided will appear in the public portion of the wrapper.
 * 
 * 
 *  @note: Requires C++-17 to function properly!
 * 
 *  @todo: Add the ability to rename functions from the base class.
 *  @todo: Add the ability to list several functions, requiring only one to exist.
 *         (This can be done by surrounding all names in parens to build a pack)
 */

#ifndef EMP_CONCEPT_WRAPPER_H
#define EMP_CONCEPT_WRAPPER_H

#include <string>
#include <utility>

#include "../base/macros.h"
#include "meta.h"

#define EMP_BUILD_CONCEPT( CLASS_NAME, ... )                         \
  /* Build the interface class. */                                   \
  class CLASS_NAME ## _Base {                                        \
  public:                                                            \
    EMP_WRAP_EACH(EMP_BUILD_CONCEPT__BASE, __VA_ARGS__)              \
  };                                                                 \
  /* Build the wrapper class. */                                     \
  template <typename WRAPPED_T>                                      \
  class CLASS_NAME : public WRAPPED_T, public CLASS_NAME ## _Base {  \
    using this_t = CLASS_NAME<WRAPPED_T>;                            \
    EMP_WRAP_EACH(EMP_BUILD_CONCEPT__PROCESS, __VA_ARGS__)           \
  }

#define EMP_BUILD_CONCEPT__BASE( CMD ) EMP_BUILD_CONCEPT_BASE__ ## CMD

#define EMP_BUILD_CONCEPT_BASE__REQUIRED_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;
#define EMP_BUILD_CONCEPT_BASE__OPTIONAL_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;
#define EMP_BUILD_CONCEPT_BASE__PRIVATE(...)
#define EMP_BUILD_CONCEPT_BASE__PROTECTED(...)
#define EMP_BUILD_CONCEPT_BASE__PUBLIC(...)


#define EMP_BUILD_CONCEPT__PROCESS( CMD ) EMP_BUILD_CONCEPT__PROCESS_ ## CMD

#define EMP_BUILD_CONCEPT__PROCESS_REQUIRED_FUN(FUN_NAME, ERROR, ...)                             \
  EMP_BUILD_CONCEPT__REQUIRED_impl(FUN_NAME, ERROR,                                               \
                                   EMP_DEC(EMP_COUNT_ARGS(__VA_ARGS__)), /* How many args? */     \
                                   EMP_GET_ARG(1, __VA_ARGS__),          /* Return type */        \
                                   EMP_POP_ARG(__VA_ARGS__) )            /* Arg types */

#define EMP_BUILD_CONCEPT__REQUIRED_impl(FUN_NAME, ERROR, NUM_ARGS, RETURN_T, ...)                \
  protected:                                                                                      \
    /* Determine return type if we try to call this function in the base class.                   \
       It should be undefined if the member functon does not exist!                           */  \
    template <typename T>                                                                         \
    using return_t_ ## FUN_NAME =                                                                 \
      EMP_IF( NUM_ARGS,                                                                           \
        decltype( std::declval<T>().FUN_NAME() );,                                                \
        decltype( std::declval<T>().FUN_NAME(EMP_TYPES_TO_VALS(__VA_ARGS__)) );                   \
      )                                                                                           \
  public:                                                                                         \
    /* Test whether function exists, based on SFINAE in using return type.                    */  \
    static constexpr bool HasFun_ ## FUN_NAME() {                                                 \
      return emp::test_type<return_t_ ## FUN_NAME, WRAPPED_T>();                                  \
    }                                                                                             \
    /* Call appropriate version of the function.  First determine if there is a non-void          \
       return type (i.e., do we return th result?) then check if the function exists in the       \
       wrapped class or should we call/return the default (otherwise).                        */  \
    EMP_IF( NUM_ARGS,                                                                             \
            RETURN_T FUN_NAME( EMP_DECLARE_ARGS(__VA_ARGS__) ),                                   \
            RETURN_T FUN_NAME( )                                                                  \
    ) {                                                                                           \
      static_assert( HasFun_ ## FUN_NAME(), ERROR " (Class: " "TBD" ")" );                        \
      if constexpr (HasFun_ ## FUN_NAME()) {                                                      \
        EMP_IF( EMP_TEST_IF_VOID(RETURN_T),                                                       \
          /* void return -> call function, but don't return result. */                            \
          { WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); },                                 \
          /* non-void return -> make sure to return result. */                                    \
          { return WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }                           \
        )                                                                                         \
      }                                                                                           \
    }


/// Macro to dynamically call an OPTIONAL function; it will call the version in the wrapped type
/// if it exists, or else return the default provided (which can be either a function to call or a
/// value.  The first two arguments are the function name and its default return.  The remaining
/// arguments in the ... must be the return type (required) and all argument types (if any exist)

#define EMP_BUILD_CONCEPT__PROCESS_OPTIONAL_FUN(NAME, DEFAULT, ...)                               \
  EMP_BUILD_CONCEPT__OPTIONAL_impl(NAME, DEFAULT,                                                 \
                                   EMP_DEC(EMP_COUNT_ARGS(__VA_ARGS__)),  /* How many args? */    \
                                   EMP_GET_ARG(1, __VA_ARGS__),           /* Return type */       \
                                   EMP_POP_ARG(__VA_ARGS__) )             /* Arg types */

#define EMP_BUILD_CONCEPT__OPTIONAL_impl(FUN_NAME, DEFAULT, NUM_ARGS, RETURN_T, ...)              \
  protected:                                                                                      \
    /* Determine return type if we try to call this function in the base class.                   \
       It should be undefined if the member functon does not exist!                           */  \
    template <typename T>                                                                         \
    using return_t_ ## FUN_NAME =                                                                 \
      EMP_IF( NUM_ARGS,                                                                           \
        decltype( std::declval<T>().FUN_NAME() );,                                                \
        decltype( std::declval<T>().FUN_NAME(EMP_TYPES_TO_VALS(__VA_ARGS__)) );                   \
      )                                                                                           \
  public:                                                                                         \
    /* Test whether function exists, based on SFINAE in using return type.                    */  \
    static constexpr bool HasFun_ ## FUN_NAME() {                                                 \
      return emp::test_type<return_t_ ## FUN_NAME, WRAPPED_T>();                                  \
    }                                                                                             \
    /* Call appropriate version of the function.  First determine if there is a non-void          \
       return type (i.e., do we return th result?) then check if the function exists in the       \
       wrapped class or should we call/return the default (otherwise).                        */  \
    EMP_IF( NUM_ARGS,                                                                             \
            RETURN_T FUN_NAME( EMP_DECLARE_ARGS(__VA_ARGS__) ),                                   \
            RETURN_T FUN_NAME( )                                                                  \
    ) {                                                                                           \
      constexpr bool has_fun = HasFun_ ## FUN_NAME();                                             \
      EMP_IF( EMP_TEST_IF_VOID(RETURN_T),                                                         \
        { /* void return -> call function, but don't return result. */                            \
          if constexpr (has_fun) { WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }           \
          else { DEFAULT; }                                                                       \
        },                                                                                        \
        { /* non-void return -> make sure to return result. */                                    \
          if constexpr (has_fun) { return WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }    \
          else { return DEFAULT; }                                                                \
        }                                                                                         \
      )                                                                                           \
    }

#define EMP_BUILD_CONCEPT__PROCESS_PRIVATE(...) private: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PUBLIC(...) public: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PROTECTED(...) protected: __VA_ARGS__


#endif

