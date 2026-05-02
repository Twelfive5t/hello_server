cmake_minimum_required(VERSION 3.16)

project(TelemetryLib)

find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(gRPC REQUIRED)

add_library(TelemetryLib STATIC telemetry.cpp)

target_include_directories(TelemetryLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TelemetryLib PUBLIC
    opentelemetry-cpp::opentelemetry-cpp
    opentelemetry_exporter_ostream_span
    opentelemetry_version
    opentelemetry_exporter_otlp_grpc
    opentelemetry_exporter_otlp_grpc_metrics
    opentelemetry_exporter_otlp_grpc_log
    opentelemetry_exporter_otlp_grpc_client
    opentelemetry_proto_grpc
    opentelemetry_proto
    opentelemetry_trace
    opentelemetry_metrics
    gRPC::grpc++
)

target_compile_features(TelemetryLib PUBLIC cxx_std_14)
