idf_component_register(SRCS "ota-dishwasher.cpp"
                    INCLUDE_DIRS ".")
set(SERVER_CERT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/server_cert.pem")
target_add_binary_data(${COMPONENT_TARGET} "${SERVER_CERT_FILE}" TEXT)
