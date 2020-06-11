
if(DEFINED IN_SOURCE_BUILD)
    set(LLVM_LINK_COMPONENTS BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support)
    add_llvm_tool( PcaMem MemCheck.cpp Main.cpp)
else()
    llvm_map_components_to_libnames(llvm_libs BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support )
    add_executable( PcaMem MemCheck.cpp Main.cpp)

    target_link_libraries( PcaMem LLVMllaf ${llvm_libs} )

    set_target_properties( PcaMem PROPERTIES
                           RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
endif()
