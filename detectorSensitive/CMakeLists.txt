#################################################################################
##Package : detectorSensitive
#################################################################################

file(GLOB sources src/*.cpp)

add_library(detectorSensitive SHARED ${sources} )
add_library(k4geo::detectorSensitive ALIAS detectorSensitive)
dd4hep_generate_rootmap(detectorSensitivePlugin)
target_link_libraries(detectorSensitive DD4hep::DDCore)
target_include_directories(detectorSensitive
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

file(GLOB headers include/detectorSensitive/*.h)
set_target_properties(detectorSensitive PROPERTIES PUBLIC_HEADER "${headers}")

file(GLOB pluginsources src/plugins/*.cpp)
add_library(detectorSensitivePlugin SHARED ${pluginsources})
target_link_libraries(detectorSensitivePlugin detectorSensitive DD4hep::DDCore)
target_include_directories(detectorSensitivePlugin
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)



# Destination directories are hardcoded because GNUdirectories are not included
install(TARGETS detectorSensitive detectorSensitivePlugin
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIDIR} COMPONENT shlib
  PUBLIC_HEADER DESTINATION include/detectorSensitive COMPONENT dev
)

