cmake_minimum_required(VERSION 3.29.5)
project(
  SML_LLVM
  VERSION 0.1.0
  DESCRIPTION "Targeting Samora Lang to LLVM"
  LANGUAGES C)

file(GLOB sml_files "src/*.c")
add_executable(sml ${sml_files})
