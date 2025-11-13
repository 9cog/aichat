cmake_minimum_required(VERSION 3.15)

# Simple test framework
add_executable(test_kernel test_kernel.c)
target_link_libraries(test_kernel PRIVATE aichat-core)

add_executable(test_cognitive test_cognitive.cpp)
target_link_libraries(test_cognitive PRIVATE aichat-core)

# Register tests
add_test(NAME kernel_bootstrap COMMAND test_kernel bootstrap)
add_test(NAME kernel_scheduler COMMAND test_kernel scheduler)
add_test(NAME kernel_memory COMMAND test_kernel memory)
add_test(NAME kernel_hgfs COMMAND test_kernel hgfs)

add_test(NAME cognitive_atomspace COMMAND test_cognitive atomspace)
add_test(NAME cognitive_ecan COMMAND test_cognitive ecan)
add_test(NAME cognitive_pln COMMAND test_cognitive pln)
add_test(NAME cognitive_esn COMMAND test_cognitive esn)
