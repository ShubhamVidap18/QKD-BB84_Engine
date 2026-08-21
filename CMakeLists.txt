cmake_minimum_required(VERSION 3.20)

project(bb84-engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Dependencies
# --------------------------------------------------

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

# ============================================================
# BB84 Engine Library
# ============================================================

add_library(bb84-engine
    src/BB84Provider.cpp
    src/PhotonGenerator.cpp
    src/OpenSSLRandomGenerator.cpp
    src/BasisSelector.cpp
    src/Alice.cpp
    src/Bob.cpp
    src/BasisSifter.cpp
    src/QuantumChannel.cpp
    src/QBER.cpp
    src/ErrorReconciliation.cpp
    src/AuthenticatedClassicalChannel.cpp
)


# Header files are located under include/
target_include_directories(bb84-engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

target_link_libraries(bb84-engine
    PUBLIC
        OpenSSL::Crypto
)

# Compiler warnings
target_compile_options(bb84-engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
)


# ============================================================
# PhotonGenerator Test
# ============================================================

add_executable(photon_generator_test
    tests/test_PhotonGenerator.cpp
)

target_link_libraries(photon_generator_test
    PRIVATE
        bb84-engine
)


# ============================================================
# OpenSSL CSPRNG tests
# ============================================================

add_executable(openssl_random_generator_test
    tests/test_OpenSSLRandomGenerator.cpp
)

target_link_libraries(openssl_random_generator_test
    PRIVATE
        bb84-engine
)


# ============================================================
# BB84 State Machine Test
# ============================================================

add_executable(bb84_state_machine_test
    tests/test_bb84_state_machine.cpp
)

target_link_libraries(bb84_state_machine_test
    PRIVATE
        bb84-engine
)


# ============================================================
# BB84 State Machine Example
# ============================================================


#add_executable(bb84_state_machine_example
#    examples/state_machine_example.cpp
#)

#target_link_libraries(bb84_state_machine_example
#    PRIVATE
#        bb84-engine)


# --------------------------------------------------
# BasisSelector test
# --------------------------------------------------

add_executable(basis_selector_test
    tests/test_BasisSelector.cpp
)

target_link_libraries(basis_selector_test
    PRIVATE
        bb84-engine
)


# --------------------------------------------------
# Alice's Part
# --------------------------------------------------

add_executable(alice_test
            tests/test_Alice.cpp
)

target_link_libraries(alice_test
            PRIVATE
                bb84-engine
)

add_executable(alice_example
            examples/alice_example.cpp
)

target_link_libraries(alice_example
    PRIVATE
        bb84-engine
)


# --------------------------------------------------
# Bob's Part
# --------------------------------------------------

add_executable(bob_test
    tests/test_Bob.cpp
)

target_link_libraries(bob_test
PRIVATE
bb84-engine)

target_include_directories(bob_test
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(bob_example
examples/bob_example.cpp
)

target_link_libraries(bob_example
PRIVATE
bb84-engine
)

target_include_directories(bob_example
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include)


# --------------------------------------------------
# Basis Sifter 
# --------------------------------------------------

add_executable(basis_sifter_test
    tests/test_BasisSifter.cpp
)

target_link_libraries(basis_sifter_test
    PRIVATE
        bb84-engine
)

add_executable(basis_sifter_example
examples/basis_sifter_example.cpp
)

target_link_libraries(basis_sifter_example
PRIVATE
bb84-engine
)

target_include_directories(basis_sifter_example
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include)


# --------------------------------------------------
# QuantumChannel  
# --------------------------------------------------

add_executable(
    quantum_channel_test
    tests/test_QuantumChannel.cpp
)

target_link_libraries(
    quantum_channel_test
    PRIVATE
    bb84-engine
)

add_executable(quantum_channel_example
examples/quantum_channel_example.cpp
)

target_link_libraries(quantum_channel_example
PRIVATE
bb84-engine
)

target_include_directories(quantum_channel_example
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include)


# ============================================================
# QBER 
# ============================================================

add_executable(
    qber_test
    tests/test_QBER.cpp
)

target_link_libraries(
    qber_test
    PRIVATE
    bb84-engine
)

add_executable(qber_example
examples/qber_example.cpp
)

target_link_libraries(qber_example
PRIVATE
bb84-engine
)

target_include_directories(qber_example
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include)


# ============================================================
# Alice to Bob Pipeline Example
# ============================================================

add_executable(alice_to_bob_pipeline_example
examples/alice_to_bob_pipeline_example.cpp
)

target_link_libraries(alice_to_bob_pipeline_example
PRIVATE
bb84-engine
)

target_include_directories(alice_to_bob_pipeline_example
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include)


# ============================================================
# Error Reconciliation
# ============================================================
find_package(GTest REQUIRED)
add_executable(
    error_reconciliation_test
    tests/test_Error_Reconciliation.cpp
)

target_link_libraries(
    error_reconciliation_test
    PRIVATE
    bb84-engine
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(error_reconciliation_test)


#add_executable(error_reconciliation_example
#examples/error_reconciliation_example.cpp
#)

#target_link_libraries(error_reconciliation_example
#PRIVATE
#bb84-engine
#)

#target_include_directories(error_reconciliation_example
#PRIVATE
#${CMAKE_CURRENT_SOURCE_DIR}/include)


# ============================================================
# Testing
# ============================================================

enable_testing()

enable_testing()

add_test(
    NAME photon_generator_test
    COMMAND photon_generator_test
)

add_test(
    NAME openssl_random_generator_test
    COMMAND openssl_random_generator_test
)

add_test(
    NAME basis_selector_test
    COMMAND basis_selector_test
)

add_test(
    NAME bb84_state_machine_test
    COMMAND bb84_state_machine_test
)

add_test(
    NAME alice_test
    COMMAND alice_test
)

add_test(
    NAME bob_test
    COMMAND bob_test
)

add_test(
    NAME basis_sifter_test
    COMMAND basis_sifter_test
)

add_test(
    NAME quantum_channel_test
    COMMAND quantum_channel_test
)

add_test(
    NAME qber_test
    COMMAND qber_test
)

add_test(
    NAME error_reconciliation_test
    COMMAND error_reconciliation_test
)
