
set(client_SRCS
   client.c
)

IF(MSVC)
set_source_files_properties(${client_SRCS}
                                       PROPERTIES LANGUAGE CXX)
ENDIF(MSVC)

add_executable(client
  ${client_SRCS}
)

target_link_libraries(client
)

