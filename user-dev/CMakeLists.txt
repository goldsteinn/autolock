cmake_minimum_required(VERSION 3.1.3)
project(Project_Template C CXX ASM)
include(ProcessorCount)


set(compiler_flags_extra "")
set(link_libraries  "")


set(third_party_libs abseil folly)

set(WITH_THIRD_PART 0)
if(${with_third_party})
  foreach(third_party_lib ${third_party_libs})
    add_subdirectory("third-party/${third_party_lib}")
  endforeach()
  foreach(third_party_lib ${third_party_libs})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third-party/${third_party_lib}")
  endforeach()
  list(APPEND compiler_flags_extra "-DI_WITH_THIRD_PARTY=1")
  # Add third party stuff
  list(APPEND link_libraries "absl::synchronization" "folly")
else()
  list(APPEND compiler_flags_extra "-DI_WITH_THIRD_PARTY=0")
endif()


set(WITH_MATH 1 CACHE BOOL "Set to load math (enables some stats stuff)")
set_property(CACHE WITH_MATH
  PROPERTY STRINGS "1" "0")

set(WITH_THREAD 1 CACHE BOOL "Set to load pthread")
set_property(CACHE WITH_THREAD
  PROPERTY STRINGS "1" "0")

set(WITH_VDSO 1 CACHE BOOL "Set to include vdso (brings in -ldl)")
set_property(CACHE WITH_VDSO
  PROPERTY STRINGS "1" "0")

set(LANG "C" CACHE STRING "Set Language (CXX or C")
set_property(CACHE LANG
  PROPERTY STRINGS "C" "CXX")


set(COMPILER "gcc" CACHE STRING "Set Language (gcc or clang")
set_property(CACHE COMPILER
  PROPERTY STRINGS "clang" "gcc")


set(LINUX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../linux-dev/src" CACHE STRING "Set path to linux")

set(cxx_lang "cxx" "c++" "cpp" "cxx")
set(c_lang "c")

set(llvm_compilers "llvm" "clang" "clang++")
set(gnu_compilers "gnu" "gcc" "gxx" "g++")

string(TOLOWER ${LANG} LANG_LOWER)
string(TOLOWER ${COMPILER} COMPILER_LOWER)

list(FIND cxx_lang ${LANG_LOWER} cxx_lang)
list(FIND c_lang ${LANG_LOWER} c_lang)

set(c_std 11)
set(cxx_std 17)

if(${cxx_lang} GREATER -1)
  set(LANG "CXX")
  set(STD "c++17") # we want if constexpr
  set(LANG_FLAGS "-fno-exceptions" "-Wno-noexcept-type" "-fno-rtti" "-Wno-ignored-attributes")
elseif(${c_lang} GREATER -1)
  set(LANG "C")
  set(STD "c11")
  set(LANG_FLAGS "")
else()
  message(FATAL_ERROR "Invalid language selection: ${LANG}")
endif()

if(${WITH_MATH})
  list(APPEND link_libraries "-lm")
  list(APPEND compiler_flags_extra "-DWITH_MATH")
endif()

if(${WITH_THREAD})
  list(APPEND link_libraries "-lpthread")
  list(APPEND compiler_flags_extra "-DWITH_THREAD")
endif()

if(${WITH_VDSO})
  list(APPEND link_libraries "-ldl")
  list(APPEND compiler_flags_extra "-DWITH_VDSO")
endif()


get_filename_component(LINUX_DIR "${LINUX_DIR}" ABSOLUTE)


if(NOT IS_DIRECTORY "${LINUX_DIR}")
  message(FATAL_ERROR "Unabled to find linux directory: ${LINUX_DIR}")
endif()
set(BIN_INSTALL_DIR "${LINUX_DIR}/user-bin")
set(LIB_INSTALL_DIR "${LINUX_DIR}/interpose-libs")



list(FIND llvm_compilers ${COMPILER_LOWER} llvm_compilers)
list(FIND gnu_compilers ${COMPILER_LOWER} gnu_compilers)
if(${llvm_compilers} GREATER -1)
  set(CMAKE_C_COMPILER "/usr/bin/clang")
  set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
elseif(${gnu_compilers} GREATER -1)
  set(CMAKE_C_COMPILER "/usr/bin/gcc")
  set(CMAKE_CXX_COMPILER "/usr/bin/g++")
else()
  message(FATAL_ERROR "Invalid compiler selection: ${COMPILER}")
endif()



set(CMAKE_EXPORT_COMPILE_COMMANDS on)


set(SRC_DIR "src")
set(FULL_SRC_DIR "${CMAKE_SOURCE_DIR}/${SRC_DIR}")

if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${FULL_SRC_DIR}/,,$(abspath $<))\"'")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${FULL_SRC_DIR}/,,$(abspath $<))\"'")
endif()

set(attrs "pure" "const" "noreturn" "format" "cold" "malloc")
set(wattrs)
foreach(attr ${attrs})
  list(APPEND wattrs "-Wsuggest-attribute=${attr}")
endforeach()

ProcessorCount(SYS_NUM_CPUS)
set(default_compiler_flags "-D_GNU_SOURCE" "-O2" "-Wwrite-strings" "-Wcast-qual" "-Wdangling-else" "-Woverlength-strings" "-Wdisabled-optimization" "-Wstrict-aliasing" "-Wstrict-overflow" "-Werror" "-Wall" "-Wextra" "-Wno-unused-function" "-Wno-variadic-macros" "-march=native" "-mtune=native" "-std=${STD}" "${LANG_FLAGS}" "${file_name_flag}" "-DI_SYS_NUM_CPUS=${SYS_NUM_CPUS}")


set(driver_compiler_flags ${default_compiler_flags})
list(APPEND driver_compiler_flags ${compiler_flags_extra})

set(test_and_bench_compiler_flags ${default_compiler_flags})
set(test_and_bench_libraries ${link_libraries})

list(APPEND test_and_bench_compiler_flags "-DWITH_THREAD" "-DWITH_MATH" "-DWITH_VDSO")
list(APPEND test_and_bench_libraries "-lpthread" "-lm" "-ldl")

set(EXE_SRC ${SRC_DIR}/driver.c)
set(EXE "driver")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}")
# Include abseil/folly



file(GLOB_RECURSE C_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.c")
set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE ${LANG})

set(ALL_C_SOURCES ${C_SOURCES})
list(REMOVE_ITEM C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${EXE_SRC}")

file(GLOB_RECURSE ASM_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.S")
list(FILTER ASM_SOURCES EXCLUDE REGEX ".*/core/.*\.S$")


# Setup test executable
list(FILTER C_SOURCES EXCLUDE REGEX ".*/test/.*$")
list(FILTER C_SOURCES EXCLUDE REGEX ".*\.test\.c$")

set(TEST_EXE_SRC "${SRC_DIR}/test/test-driver.c")
set(TEST_EXE "test-driver")
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.test.c")
file(GLOB_RECURSE TEST_UTIL_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/test/*.c")
set(TEST_C_SOURCES ${TEST_SOURCES} ${TEST_UTIL_SOURCES})
list(REMOVE_ITEM TEST_C_SOURCES ${TEST_EXE_SRC})


# Setup bench executable
list(FILTER C_SOURCES EXCLUDE REGEX ".*/bench/.*$")
list(FILTER C_SOURCES EXCLUDE REGEX ".*\.bench\.c$")

set(BENCH_EXE_SRC "${SRC_DIR}/bench/bench-driver.c")
set(BENCH_EXE "bench-driver")
file(GLOB_RECURSE BENCH_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.bench.c")
file(GLOB_RECURSE BENCH_UTIL_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/bench/*.c")
set(BENCH_C_SOURCES ${BENCH_SOURCES} ${BENCH_UTIL_SOURCES})
list(REMOVE_ITEM BENCH_C_SOURCES ${BENCH_EXE_SRC})


# Setup interpose executable
list(FILTER C_SOURCES EXCLUDE REGEX ".*/interpose/.*$")
list(FILTER C_SOURCES EXCLUDE REGEX ".*\.interpose\.c$")


file(GLOB_RECURSE INTERPOSE_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/*.interpose.c")
file(GLOB_RECURSE INTERPOSE_UTIL_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/interpose/*.c")
file(GLOB_RECURSE AUTOLOCK_SOURCES "${CMAKE_SOURCE_DIR}/${SRC_DIR}/autolock-impls/*.c")
#set(INTERPOSE_C_SOURCES ${INTERPOSE_UTIL_SOURCES})

find_program(CLANG_TIDY_EXE clang-tidy NAMES clang-tidy-13 clang-tidy-12 clang-tidy-10 clang-tidy-9 clang-tidy-8)
# We don't get cache with this method which slows down check.
add_custom_target(
  static-analysis
  COMMAND ${CLANG_TIDY_EXE}
  ${ALL_C_SOURCES}
  -p=${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(driver ${EXE_SRC} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(driver PRIVATE ${driver_compiler_flags})
target_link_libraries(driver ${link_libraries})
set_target_properties(driver PROPERTIES LINKER_LANGUAGE ${LANG} CXX_STANDARD_REQUIRED ${cxx_std} CXX_STANDARD ${cxx_std} C_STANDARD_REQUIRED ${c_std} C_STANDARD ${c_std})



set(CMAKE_INSTALL_PREFIX "${BIN_INSTALL_DIR}")
install(TARGETS driver DESTINATION "${BIN_INSTALL_DIR}")

add_executable(tests ${TEST_EXE_SRC} ${TEST_C_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(tests PRIVATE ${test_and_bench_compiler_flags})
target_link_libraries(tests ${test_and_bench_libraries})
set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE LINKER_LANGUAGE ${LANG} CXX_STANDARD_REQUIRED ${cxx_std} CXX_STANDARD ${cxx_std} C_STANDARD_REQUIRED ${c_std} C_STANDARD ${c_std})


add_executable(bench ${BENCH_EXE_SRC} ${BENCH_C_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(bench PRIVATE ${test_and_bench_compiler_flags})
target_link_libraries(bench ${test_and_bench_libraries})
set_target_properties(bench PROPERTIES EXCLUDE_FROM_ALL TRUE LINKER_LANGUAGE ${LANG} CXX_STANDARD_REQUIRED ${cxx_std} CXX_STANDARD ${cxx_std} C_STANDARD_REQUIRED ${c_std} C_STANDARD ${c_std})


set(interpose_lib_compiler_flags ${driver_compiler_flags})
list(APPEND interpose_lib_compiler_flags "-ftls-model=initial-exec")
list(APPEND interpose_lib_compiler_flags "-DI_USE_FOR_SHARED_LIBRARY_INTERPOSE")

set(interpose_lib_profile_compiler_flags ${interpose_lib_compiler_flags})
list(APPEND interpose_lib_profile_compiler_flags "-DI_WITH_TIMING")


foreach(INTERPOSE_SOURCE ${INTERPOSE_SOURCES})
  get_filename_component(INTERPOSE_LIB "${INTERPOSE_SOURCE}" NAME_WE)
  add_library(${INTERPOSE_LIB} SHARED ${INTERPOSE_SOURCE} ${INTERPOSE_C_SOURCES})
  target_compile_options(${INTERPOSE_LIB} PRIVATE ${interpose_lib_compiler_flags})
  set_target_properties(${INTERPOSE_LIB} PROPERTIES LINKER_LANGUAGE C C_STANDARD_REQUIRED ${c_std} C_STANDARD ${c_std})
  target_link_libraries(${INTERPOSE_LIB} "-ldl")
  install(TARGETS ${INTERPOSE_LIB} DESTINATION "${LIB_INSTALL_DIR}")

  set(INTERPOSE_LIB_PROFILE "${INTERPOSE_LIB}-profile")
  add_library(${INTERPOSE_LIB_PROFILE} SHARED ${INTERPOSE_SOURCE} ${INTERPOSE_C_SOURCES})
  target_compile_options(${INTERPOSE_LIB_PROFILE} PRIVATE ${interpose_lib_profile_compiler_flags})
  set_target_properties(${INTERPOSE_LIB_PROFILE} PROPERTIES LINKER_LANGUAGE C C_STANDARD_REQUIRED ${c_std} C_STANDARD ${c_std})
  target_link_libraries(${INTERPOSE_LIB_PROFILE} "-ldl")
  install(TARGETS ${INTERPOSE_LIB_PROFILE} DESTINATION "${LIB_INSTALL_DIR}")
endforeach()


add_custom_target(full)
add_dependencies(full driver tests bench)
