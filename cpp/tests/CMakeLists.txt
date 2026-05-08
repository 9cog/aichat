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

add_executable(test_rag test_rag.cpp)
target_link_libraries(test_rag PRIVATE aichat-core)

add_executable(test_function test_function.cpp)
target_link_libraries(test_function PRIVATE aichat-core)

# Register tests
add_test(NAME kernel_bootstrap COMMAND test_kernel bootstrap)
add_test(NAME kernel_scheduler COMMAND test_kernel scheduler)
add_test(NAME kernel_memory COMMAND test_kernel memory)
add_test(NAME kernel_hgfs COMMAND test_kernel hgfs)
add_test(NAME kernel_sync COMMAND test_kernel sync)
add_test(NAME kernel_timer COMMAND test_kernel timer)

add_test(NAME cognitive_atomspace COMMAND test_cognitive atomspace)
add_test(NAME cognitive_ecan COMMAND test_cognitive ecan)
add_test(NAME cognitive_pln COMMAND test_cognitive pln)
add_test(NAME cognitive_esn COMMAND test_cognitive esn)

add_test(NAME cogloop_create COMMAND test_cogloop create)
add_test(NAME cogloop_cycle COMMAND test_cogloop cycle)
add_test(NAME cogloop_query COMMAND test_cogloop query)

add_test(NAME session_create COMMAND test_session create)
add_test(NAME session_persist COMMAND test_session persist)

add_test(NAME rag_create COMMAND test_rag create)
add_test(NAME rag_add_text COMMAND test_rag add_text)
add_test(NAME rag_query COMMAND test_rag query)
add_test(NAME rag_build_context COMMAND test_rag build_context)
add_test(NAME rag_add_file COMMAND test_rag add_file)

add_test(NAME function_registry COMMAND test_function registry)
add_test(NAME function_register COMMAND test_function register)
add_test(NAME function_tools_prompt COMMAND test_function tools_prompt)
add_test(NAME function_dispatch COMMAND test_function dispatch)
add_test(NAME function_has_tool_call COMMAND test_function has_tool_call)
add_test(NAME function_shell_tool COMMAND test_function shell_tool)
add_test(NAME function_failing_cb COMMAND test_function failing_cb)
