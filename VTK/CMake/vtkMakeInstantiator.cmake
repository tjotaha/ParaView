#
# a cmake implementation of the Wrap Tcl command
#

# VTK_MAKE_INSTANTIATOR(className 
#                       outSourceList
#                       src-list1
#                       EXPORT_MACRO
#                       HEADER_LOCATION
#                       INCLUDES)
#
# Generates a new class with the given name and adds its files to the
# given outSourceList.  It registers the classes from the other given
# source lists with vtkInstantiator when it is loaded.  The output
# source list should be added to the library with the classes it
# registers.
# The EXPORT_MACRO argument must be given and followed by the export
# macro to use when generating the class (ex. VTK_COMMON_EXPORT).
# The HEADER_LOCATION option must be followed by a path.  It specifies
# the directory in which to place the generated class's header file.
# The generated class implementation files always go in the build
# directory corresponding to the CMakeLists.txt file containing
# the command.  This is the default location for the header.
# The INCLUDES option can be followed by a list of zero or more files.
# These files will be #included by the generated instantiator header,
# and can be used to gain access to the specified exportMacro in the
# C++ code.

MACRO(VTK_MAKE_INSTANTIATOR3 className outSourceList SOURCES EXPORT_MACRO HEADER_LOCATION INCLUDES)

  # make the arguments available to the configured files
  SET (VTK_MAKE_INSTANTIATOR_CLASS_NAME ${className})
  STRING(TOUPPER ${EXPORT_MACRO} VTK_MAKE_INSTANTIATOR_EXPORT_MACRO)

  # For each include
  FOREACH(FILE ${INCLUDES})
    # generate the header
    SET (HEADER_CONTENTS 
      "${HEADER_CONTENTS}#include \"${FILE}\"\n")
  ENDFOREACH(FILE)

  # For each class
  FOREACH(FILE ${SOURCES})

    # should we wrap the file?
    SET (WRAP_THIS_CLASS 1)
    GET_SOURCE_FILE_PROPERTY(TMP_WRAP_EXCLUDE ${FILE} WRAP_EXCLUDE)
    GET_SOURCE_FILE_PROPERTY(TMP_ABSTRACT ${FILE} ABSTRACT)

    # if it is abstract or wrap exclude then don't wrap it
    IF (TMP_WRAP_EXCLUDE OR TMP_ABSTRACT)
      SET (WRAP_THIS_CLASS 0)
    ENDIF (TMP_WRAP_EXCLUDE OR TMP_ABSTRACT)
    
    # don't wrap vtkIndent or vtkTimeStamp
    IF (${FILE} MATCHES "vtkIndent")
      SET (WRAP_THIS_CLASS 0)
    ENDIF (${FILE} MATCHES "vtkIndent")
    IF (${FILE} MATCHES "vtkTimeStamp")
      SET (WRAP_THIS_CLASS 0)
    ENDIF (${FILE} MATCHES "vtkTimeStamp")
      
    # finally if we should wrap it, then ...
    IF (WRAP_THIS_CLASS)

      # what is the filename without the extension
      GET_FILENAME_COMPONENT(TMP_FILENAME ${FILE} NAME_WE)

      # generate the implementation
      SET (CXX_CONTENTS 
        "${CXX_CONTENTS}extern vtkObject* vtkInstantiator${TMP_FILENAME}New();\n")

      SET (CXX_CONTENTS2 
        "${CXX_CONTENTS2}  vtkInstantiator::RegisterInstantiator(\"${TMP_FILENAME}\", vtkInstantiator${TMP_FILENAME}New);\n")

      SET (CXX_CONTENTS3 
        "${CXX_CONTENTS3}  vtkInstantiator::UnRegisterInstantiator(\"${TMP_FILENAME}\", vtkInstantiator${TMP_FILENAME}New);\n")

    ENDIF (WRAP_THIS_CLASS)
  ENDFOREACH(FILE)

  # add the source file to the source list
  SET(${outSourceList} ${${outSourceList}} 
    ${CMAKE_CURRENT_BINARY_DIR}/${className}.cxx)

  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_CURRENT_BINARY_DIR}/${className}.cxx 
    PROPERTIES GENERATED 1 WRAP_EXCLUDE 1 ABSTRACT 0
    )


  IF (NOT VTK_CMAKE_DIR)
    SET(VTK_CMAKE_DIR ${VTK_SOURCE_DIR}/CMake)
  ENDIF (NOT VTK_CMAKE_DIR)

  CONFIGURE_FILE(
    ${VTK_CMAKE_DIR}/vtkMakeInstantiator.h.in  
    ${HEADER_LOCATION}/${className}.h
    COPY_ONLY
    IMMEDIATE
    )
  CONFIGURE_FILE(
    ${VTK_CMAKE_DIR}/vtkMakeInstantiator.cxx.in  
    ${CMAKE_CURRENT_BINARY_DIR}/${className}.cxx
    COPY_ONLY
    IMMEDIATE
    )
  
ENDMACRO(VTK_MAKE_INSTANTIATOR3)
