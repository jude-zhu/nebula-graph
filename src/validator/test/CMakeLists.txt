# Copyright (c) 2020 vesoft inc. All rights reserved.
#
# This source code is licensed under Apache 2.0 License,
# attached with Common Clause Condition 1.0, found in the LICENSES directory.

nebula_add_library(
    mock_schema_obj OBJECT
    MockSchemaManager.cpp
    MockIndexManager.cpp
)

set(VALIDATOR_TEST_LIBS
    $<TARGET_OBJECTS:mock_schema_obj>
    $<TARGET_OBJECTS:util_obj>
    $<TARGET_OBJECTS:validator_obj>
    $<TARGET_OBJECTS:expr_visitor_obj>
    $<TARGET_OBJECTS:planner_obj>
    $<TARGET_OBJECTS:session_obj>
    $<TARGET_OBJECTS:graph_flags_obj>
    $<TARGET_OBJECTS:parser_obj>
    $<TARGET_OBJECTS:idgenerator_obj>
    $<TARGET_OBJECTS:context_obj>
    $<TARGET_OBJECTS:graph_auth_obj>
    $<TARGET_OBJECTS:common_expression_obj>
    $<TARGET_OBJECTS:common_network_obj>
    $<TARGET_OBJECTS:common_fs_obj>
    $<TARGET_OBJECTS:common_time_obj>
    $<TARGET_OBJECTS:common_stats_obj>
    $<TARGET_OBJECTS:common_time_obj>
    $<TARGET_OBJECTS:common_common_thrift_obj>
    $<TARGET_OBJECTS:common_graph_thrift_obj>
    $<TARGET_OBJECTS:common_storage_thrift_obj>
    $<TARGET_OBJECTS:common_thrift_obj>
    $<TARGET_OBJECTS:common_thread_obj>
    $<TARGET_OBJECTS:common_datatypes_obj>
    $<TARGET_OBJECTS:common_base_obj>
    $<TARGET_OBJECTS:common_meta_thrift_obj>
    $<TARGET_OBJECTS:common_meta_obj>
    $<TARGET_OBJECTS:common_graph_thrift_obj>
    $<TARGET_OBJECTS:common_charset_obj>
    $<TARGET_OBJECTS:common_meta_client_obj>
    $<TARGET_OBJECTS:common_file_based_cluster_id_man_obj>
    $<TARGET_OBJECTS:common_function_manager_obj>
    $<TARGET_OBJECTS:common_agg_function_manager_obj>
    $<TARGET_OBJECTS:common_conf_obj>
    $<TARGET_OBJECTS:common_encryption_obj>
    $<TARGET_OBJECTS:common_http_client_obj>
    $<TARGET_OBJECTS:common_process_obj>
    $<TARGET_OBJECTS:common_time_utils_obj>
    $<TARGET_OBJECTS:common_graph_obj>
    $<TARGET_OBJECTS:common_ft_es_graph_adapter_obj>
    $<TARGET_OBJECTS:common_ws_common_obj>
)

nebula_add_test(
    NAME
        validator_test
    SOURCES
        ACLValidatorTest.cpp
        FetchVerticesTest.cpp
        FetchEdgesTest.cpp
        MockSchemaManagerTest.cpp
        QueryValidatorTest.cpp
        AdminValidatorTest.cpp
        MaintainValidatorTest.cpp
        MutateValidatorTest.cpp
        YieldValidatorTest.cpp
        GetSubgraphValidatorTest.cpp
        FindPathValidatorTest.cpp
        ValidatorTestBase.cpp
        ExplainValidatorTest.cpp
        GroupByValidatorTest.cpp
        MatchValidatorTest.cpp
        LookupValidatorTest.cpp
        SymbolsTest.cpp
    OBJECTS
        ${VALIDATOR_TEST_LIBS}
    LIBRARIES
        gtest
        ${THRIFT_LIBRARIES}
        ${PROXYGEN_LIBRARIES}
)
