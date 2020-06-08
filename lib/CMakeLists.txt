
# Due to a mutual dependencies, all the sub projects of the SVG are merged here
# Otherwise it is impossible to load the dependencies in opt
# NOTE: if the llaf should be linked into opt, we should probably use the individual sub projects here, rather than the combined project 
set(SOURCES
    common/VisitDir.cpp
	common/MultiTask.cpp
	common/SoftPara.cpp
	common/Stat.cpp
	callgraph/CallGraph.cpp
	llvmadpt/LlvmAdpt.cpp
	llvmadpt/ModuleSet.cpp
	analysis/Analysis.cpp
	analysis/points-to/PointsTo.cpp
	analysis/points-to/Anderson.cpp
	analysis/Dependence.cpp
	analysis/ExternalLib.cpp
	analysis/ProgramSlice.cpp
	app/leakdetect/MemLeak.cpp
	app/leakdetect/MemFunctions.cpp
    )

add_llvm_loadable_module(llaf ${SOURCES})
add_llvm_Library(LLVMllaf ${SOURCES})

llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support )
target_link_libraries(LLVMllaf ${llvm_libs})

target_link_libraries(llaf ${llvm_libs})

if(DEFINED IN_SOURCE_BUILD)
    add_dependencies(llaf intrinsics_gen)
    add_dependencies(LLVMllaf intrinsics_gen)
endif()
