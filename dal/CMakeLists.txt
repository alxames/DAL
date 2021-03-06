include(TestSizeT)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/size_types.i.in"
  "${CMAKE_CURRENT_BINARY_DIR}/size_types.i"
)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/dal_config.h
  dal_version.h

  DESTINATION include/dal
  COMPONENT headers
)

set(dal_sources
  dal_version.cc

  hdf5/File.cc
  hdf5/Group.cc
  hdf5/Node.cc
  hdf5/exceptions/exceptions.cc
  hdf5/exceptions/errorstack.cc
  hdf5/types/FileInfo.cc
  hdf5/types/versiontype.cc

  lofar/Flagging.cc
  lofar/StationNames.cc
  lofar/BF_File.cc
  lofar/CLA_File.cc
  lofar/Coordinates.cc
  lofar/TBB_File.cc
)

set(dal_headers
  dal_version.h

  hdf5/Attribute.h
  hdf5/Attribute.tcc
  hdf5/exceptions/errorstack.h
  hdf5/exceptions/exceptions.h
  hdf5/File.h
  hdf5/Dataset.h
  hdf5/Dataset.tcc
  hdf5/Group.h
  hdf5/types/FileInfo.h
  hdf5/types/h5complex.h
  hdf5/types/issame.h
  hdf5/types/implicitdowncast.h
  hdf5/types/h5typemap.h
  hdf5/types/h5tuple.h
  hdf5/types/isderivedfrom.h
  hdf5/types/versiontype.h
  hdf5/types/hid_gc.h
  hdf5/Node.h

  lofar/StationNames.h
  lofar/Flagging.h
  lofar/Coordinates.h
  lofar/TBB_File.h
  lofar/CommonTuples.h
  lofar/CLA_File.h
  lofar/BF_File.h

  casa/CasaTBBFileExtend.h
  casa/CasaDatasetExtend.h
)

set(swig_sources
  dal.i
  vectors.i
  external/numpy.i

  hdf5/Attribute.i
  hdf5/Group.i
  hdf5/exceptions/exceptions.i
  hdf5/Node.i
  hdf5/Dataset.i
  hdf5/types/h5tuple.i
  hdf5/types/versiontype.i

  lofar/TBB_File.i
  lofar/CommonTuples.i
  lofar/Flagging.i

  # *.swg files need to be present in the binary dir, as they are not included but simply checked for presence by SWIG
  ${CMAKE_CURRENT_BINARY_DIR}/pyfragments.swg
)  

add_library (lofardal
  ${dal_sources}
)

# Build version.
set_target_properties(lofardal PROPERTIES VERSION ${DAL_VERSION_STRING})

# Our API version is major.minor.
# To link to .major.minor, pass '-Wl,-soname,liblofardal.so.M.m' to GCC, or '-soname liblofardal.so.M.m' to its linker,
# although trying with just -llofardal -lhdf5 seems to work already (better).
# In the end it matters little, because all users of DAL .tcc (C++ template) files (about everyone) must (and can) recompile.
# If passed data structures change size, the dyn linker will refuse and recompilation is also needed.
set_target_properties(lofardal PROPERTIES SOVERSION "${DAL_VERSION_MAJOR}.${DAL_VERSION_MINOR}")

if(CMAKE_COMPILER_IS_GNUCXX)
  set_target_properties(lofardal PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wno-unused-function -Wno-long-long -ansi -pedantic")
endif(CMAKE_COMPILER_IS_GNUCXX)

target_link_libraries(lofardal ${HDF5_LIBRARIES})

install (TARGETS
  lofardal

  DESTINATION lib
  COMPONENT libraries
)

add_subdirectory(doc)

# Python bindings through SWIG
if(PYTHON_BINDINGS)
  # .swg files are not included, and need to be present in the target directory to be used
  add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pyfragments.swg
   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/pyfragments.swg ${CMAKE_CURRENT_BINARY_DIR}/pyfragments.swg
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pyfragments.swg
  )

  list(APPEND CMAKE_SWIG_FLAGS -Wall -Wextra)
  set_source_files_properties(dal.i PROPERTIES CPLUSPLUS ON)
  set_source_files_properties(dal.i PROPERTIES SWIG_FLAGS -modern -oldrepr -keyword -castmode -modernargs)
  set(SWIG_MODULE_dal_EXTRA_DEPS lofardal ${CMAKE_CURRENT_BINARY_DIR}/doc/docstrings.i ${swig_sources} ${dal_headers})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/doc/docstrings.i PROPERTIES GENERATED ON)
  swig_add_module(dal python dal.i ${dal_sources})
  swig_link_libraries(dal ${PYTHON_LIBRARIES} ${HDF5_LIBRARIES})

  add_dependencies(${SWIG_MODULE_dal_REAL_NAME} swig_docstrings)

  install(TARGETS
    ${SWIG_MODULE_dal_REAL_NAME}

    DESTINATION ${PYTHON_SITE_PACKAGES}/dal
    COMPONENT python
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dal.py

    DESTINATION ${PYTHON_SITE_PACKAGES}/dal
    COMPONENT python
  )
endif(PYTHON_BINDINGS)

add_subdirectory(casa)
add_subdirectory(hdf5)
add_subdirectory(lofar)
add_subdirectory(utils)

# Generate the configuration file at the end, once all options are known
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/dal_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dal_config.h"
)

