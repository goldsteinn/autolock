cmake_minimum_required(VERSION 3.1.3)
project(Project_Template C CXX ASM)
include(ProcessorCount)

set(third_party_libs abseil folly)

foreach(third_party_lib ${third_party_libs})
  add_subdirectory("third-party/${third_party_lib}")
endforeach()
foreach(third_party_lib ${third_party_libs})
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third-party/${third_party_lib}")
endforeach()



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


set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../linux-dev/src/user-bin" CACHE STRING "Set install path")

set(cxx_lang "cxx" "c++" "cpp" "cxx")
set(c_lang "c")

set(llvm_compilers "llvm" "clang" "clang++")
set(gnu_compilers "gnu" "gcc" "gxx" "g++")

string(TOLOWER ${LANG} LANG_LOWER)
string(TOLOWER ${COMPILER} COMPILER_LOWER)

list(FIND cxx_lang ${LANG_LOWER} cxx_lang)
list(FIND c_lang ${LANG_LOWER} c_lang)
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

set(compiler_flags_extra "")
set(link_libraries  "")

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


get_filename_component(INSTALL_DIR "${INSTALL_DIR}" ABSOLUTE)


if(NOT IS_DIRECTORY "${INSTALL_DIR}")
  get_filename_component(INSTALL_DIR2 "${INSTALL_DIR}/.." ABSOLUTE)
  # We can build the directory at install
  if(NOT IS_DIRECTORY "${INSTALL_DIR2}")
    message(FATAL_ERROR "Unabled to find install directory: ${INSTALL_DIR}")
  endif()
endif()


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

ProcessorCount(SYS_NUM_CPUS)
list(APPEND compiler_flags_extra "-DI_SYS_NUM_CPUS=${SYS_NUM_CPUS}")


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

set(default_compiler_flags "-D_GNU_SOURCE" "-O2" "-Wwrite-strings" "-Wcast-qual" "-Wdangling-else" "-Woverlength-strings" "-Wdisabled-optimization" "-Wstrict-aliasing" "-Wstrict-overflow" "-Werror" "-Wall" "-Wextra" "-Wno-unused-function" "-Wno-variadic-macros" "-march=native" "-mtune=native" "-std=${STD}" "${LANG_FLAGS}" "${file_name_flag}")


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



find_program(CLANG_TIDY_EXE clang-tidy NAMES clang-tidy-13 clang-tidy-12 clang-tidy-10 clang-tidy-9 clang-tidy-8)
# We don't get cache with this method which slows down check.
add_custom_target(
  static-analysis
  COMMAND ${CLANG_TIDY_EXE}
  ${ALL_C_SOURCES}
  -p=${CMAKE_CURRENT_BINARY_DIR}
  #  -checks=*,-*insecureAPI.strcpy,-*insecureAPI.DeprecatedOrUnsafeBufferHandling
)

add_executable(driver ${EXE_SRC} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(driver PRIVATE ${driver_compiler_flags})
target_link_libraries(driver ${link_libraries} absl::synchronization folly)
set_target_properties(driver PROPERTIES LINKER_LANGUAGE ${LANG})
message("Install at: ${INSTALL_DIR}")
install(TARGETS driver DESTINATION "${INSTALL_DIR}")

add_executable(tests ${TEST_EXE_SRC} ${TEST_C_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(tests PRIVATE ${test_and_bench_compiler_flags})
target_link_libraries(tests ${test_and_bench_libraries})
set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(tests PROPERTIES LINKER_LANGUAGE ${LANG})

add_executable(bench ${BENCH_EXE_SRC} ${BENCH_C_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
target_compile_options(bench PRIVATE ${test_and_bench_compiler_flags})
target_link_libraries(bench ${test_and_bench_libraries})
set_target_properties(bench PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(bench PROPERTIES LINKER_LANGUAGE ${LANG})

add_custom_target(full)
add_dependencies(full driver tests bench)
