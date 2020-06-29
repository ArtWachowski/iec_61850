add_subdirectory(server_example_config_file)
add_subdirectory(server_example_write_handler)
add_subdirectory(server_app)
add_subdirectory(server)

add_subdirectory(client)
add_subdirectory(iec61850_client_example_files)

if (NOT WIN32)
    add_subdirectory(mms_utility)
endif(NOT WIN32)

if(WIN32)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/winpcap/Lib/wpcap.lib")
        set(BUILD_SV_GOOSE_EXAMPLES ON)
        message("Found winpcap -> compile examples for GOOSE and SV")
    endif()
else()
    set(BUILD_SV_GOOSE_EXAMPLES ON)
endif()

