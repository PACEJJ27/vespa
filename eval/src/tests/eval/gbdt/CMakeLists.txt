# Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
vespa_add_executable(eval_gbdt_test_app TEST
    SOURCES
    gbdt_test.cpp
    DEPENDS
    vespaeval
)
vespa_add_test(NAME eval_gbdt_test_app COMMAND eval_gbdt_test_app)
vespa_add_executable(eval_gbdt_benchmark_app
    SOURCES
    gbdt_benchmark.cpp
    DEPENDS
    vespaeval
)
vespa_add_test(NAME eval_gbdt_benchmark_app COMMAND eval_gbdt_benchmark_app BENCHMARK)
