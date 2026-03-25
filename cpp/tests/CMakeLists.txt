cmake_minimum_required(VERSION 3.15)

# Simple test framework
add_executable(test_kernel test_kernel.c)
target_link_libraries(test_kernel PRIVATE aichat-core)

add_executable(test_cognitive test_cognitive.cpp)
target_link_libraries(test_cognitive PRIVATE aichat-core)

add_executable(test_cogloop test_cogloop.cpp)
target_link_libraries(test_cogloop PRIVATE aichat-core)

add_executable(test_session test_session.cpp)
target_link_libraries(test_session PRIVATE aichat-core)

# Register tests
add_test(NAME kernel_bootstrap COMMAND test_kernel bootstrap)
add_test(NAME kernel_scheduler COMMAND test_kernel scheduler)
add_test(NAME kernel_memory COMMAND test_kernel memory)
add_test(NAME kernel_hgfs COMMAND test_kernel hgfs)

add_test(NAME cognitive_atomspace COMMAND test_cognitive atomspace)
add_test(NAME cognitive_ecan COMMAND test_cognitive ecan)
add_test(NAME cognitive_pln COMMAND test_cognitive pln)
add_test(NAME cognitive_esn COMMAND test_cognitive esn)

add_test(NAME cogloop_create COMMAND test_cogloop create)
add_test(NAME cogloop_cycle COMMAND test_cogloop cycle)
add_test(NAME cogloop_query COMMAND test_cogloop query)

add_test(NAME session_create COMMAND test_session create)
add_test(NAME session_persist COMMAND test_session persist)
