# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\qserial_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\qserial_autogen.dir\\ParseCache.txt"
  "qserial_autogen"
  )
endif()
