cmake_minimum_required(VERSION 3.29.5)
project(
  SML_LLVM
  VERSION 0.1.0
  DESCRIPTION "Targeting Samora Lang to LLVM"
  LANGUAGES C)

file(GLOB sml_files "src/*.c")
add_executable(sml ${sml_files})

execute_process(
  COMMAND llvm-config --cflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND llvm-config --libs core
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_options(sml PRIVATE ${LLVM_CFLAGS})
target_link_libraries(sml PRIVATE ${LLVM_LIBS})
