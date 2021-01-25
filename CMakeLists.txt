# This is the root ITK CMakeLists file.
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


# This project is designed to be built outside the Insight source tree.
PROJECT(CompositITKDeformationField)

# Find ITK.
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

ADD_EXECUTABLE(CompositITKDeformationField CompositITKDeform.cpp)

#TARGET_LINK_LIBRARIES(CompositITKDeformationField ITKCommon ITKIO ITKAlgorithms)
TARGET_LINK_LIBRARIES(CompositITKDeformationField ${ITK_LIBRARIES})
