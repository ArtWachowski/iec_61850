add_subdirectory(server_app)
add_subdirectory(client)
add_subdirectory(mms_utility)

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

if(WITH_MBEDTLS)
add_subdirectory(tls_client_example)
add_subdirectory(tls_server_example)
endif(WITH_MBEDTLS)

if(${BUILD_SV_GOOSE_EXAMPLES})
    add_subdirectory(server_example_goose)
    add_subdirectory(goose_subscriber)
    add_subdirectory(goose_publisher)
    add_subdirectory(sv_subscriber)
    add_subdirectory(iec61850_9_2_LE_example)
    add_subdirectory(iec61850_sv_client_example)
    add_subdirectory(sv_publisher)
endif()


