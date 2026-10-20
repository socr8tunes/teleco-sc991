cmake_minimum_required(VERSION 3.16)
project(teleco_calc VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(TELECO_WERROR "Treat compiler warnings as errors" ON)
option(TELECO_SANITIZE "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
set(TELECO_DIFF_COUNT 20000 CACHE STRING "Differential test vectors per operation")
set(TELECO_DIFF_SEED 1 CACHE STRING "Differential test random seed")

function(teleco_target_options tgt)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${tgt} PRIVATE
      -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes)
    if(TELECO_WERROR)
      target_compile_options(${tgt} PRIVATE -Werror)
    endif()
    if(TELECO_SANITIZE)
      target_compile_options(${tgt} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
      target_link_options(${tgt} PRIVATE -fsanitize=address,undefined)
    endif()
  endif()
endfunction()

# ---- portable core -------------------------------------------------------
add_library(teleco_numeric STATIC
  core/numeric/wide.c
  core/numeric/decimal.c
  core/numeric/arithmetic.c
  core/numeric/xnum.c
  core/numeric/xconst.c
  core/functions/xfunc.c
  core/functions/mathfn.c
  core/values/exact.c
  core/values/num.c
  core/values/val.c
  core/values/cplx.c
  core/state/ctx.c
  core/calc/errors.c
  core/calc/format.c
  core/calc/nummethods.c
  core/calc/eval.c
  core/parser/lexer.c
  core/parser/expr.c)
target_include_directories(teleco_numeric PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)
teleco_target_options(teleco_numeric)

# ---- tests ---------------------------------------------------------------
enable_testing()

add_executable(test_decimal tests/unit/test_decimal.c)
target_link_libraries(test_decimal PRIVATE teleco_numeric)
teleco_target_options(test_decimal)
add_test(NAME decimal_unit COMMAND test_decimal)

add_executable(test_exact tests/unit/test_exact.c)
target_link_libraries(test_exact PRIVATE teleco_numeric)
teleco_target_options(test_exact)
add_test(NAME exact_unit COMMAND test_exact)

add_executable(dec_difftest tests/differential/dec_difftest.c)
target_link_libraries(dec_difftest PRIVATE teleco_numeric)
teleco_target_options(dec_difftest)

add_executable(fn_difftest tests/differential/fn_difftest.c)
target_link_libraries(fn_difftest PRIVATE teleco_numeric)
teleco_target_options(fn_difftest)

add_executable(bench_decimal tests/bench/bench_decimal.c)
target_link_libraries(bench_decimal PRIVATE teleco_numeric)
teleco_target_options(bench_decimal)

file(GLOB TELECO_REGRESSION_VECTORS ${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/*.vec)
if(TELECO_REGRESSION_VECTORS)
  add_test(NAME decimal_regression COMMAND dec_difftest ${TELECO_REGRESSION_VECTORS})
endif()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME decimal_differential
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/differential/run_diff.py
            --runner $<TARGET_FILE:dec_difftest>
            --count ${TELECO_DIFF_COUNT} --seed ${TELECO_DIFF_SEED})
  add_test(NAME function_differential
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/differential/run_fn_diff.py
            --runner $<TARGET_FILE:fn_difftest>
            --count 1500 --seed ${TELECO_DIFF_SEED})
else()
  message(WARNING "Python3 not found: differential tests disabled")
endif()

# ---- PC command line ------------------------------------------------------
add_library(pc_script STATIC platform/pc/calc_script.c)
target_link_libraries(pc_script PUBLIC teleco_numeric)
target_include_directories(pc_script PUBLIC platform/pc)
teleco_target_options(pc_script)

add_executable(teleco_cli platform/pc/teleco_cli.c)
target_link_libraries(teleco_cli PRIVATE pc_script)
teleco_target_options(teleco_cli)

add_executable(test_script tests/unit/test_script.c)
target_link_libraries(test_script PRIVATE pc_script)
teleco_target_options(test_script)
file(GLOB TELECO_CASE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/*.txt)
foreach(cf ${TELECO_CASE_FILES})
  get_filename_component(cn ${cf} NAME_WE)
  add_test(NAME cases_${cn} COMMAND test_script ${cf})
endforeach()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME expression_differential
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/differential/run_expr_diff.py
            --cli $<TARGET_FILE:teleco_cli> --count 1500 --seed ${TELECO_DIFF_SEED})
endif()
