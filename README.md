# PCA

README: PCA: Memory Leak Detection using Partial Call-Path Analysis


Step 1: Compile LLVM 7.0 with golden plugin.

<1> Download 

    get LLVM: http://releases.llvm.org/download.html#7.0.0
    
    get binutils: http://ftp.gnu.org/gnu/binutils/

<2> Compile

    [compile binutils]:
    
	mkdir build
	
        cd build
	
        ../binutils/configure --enable-gold --enable-plugins --disable-werro
	
        make all-gold
		
    [compile llvm]:
    
		mkdir build
		
		cd build
		
		cmake -DCMAKE_BUILD_TYPE:String=Release -DLLVM_BINUTILS_INCDIR=../binutils-2.32/include ../llvm
		


Step 2: Compile PCA and application tools based on it.

<1> Configure environment
	#The following three configuration could be added into "/etc/profile"
	
	export LLVM_PATH=/your_path/llvm7.0.0
	
	export CLANG_PATH=$LLVM_PATH/build/bin
	
	export PATH=$PATH:$CLANG_PATH
	

<2> build PCA

	cd /your_path/PCA
	
	./build.sh
	
	#the executable of tool would be generated to build/bin
	


Step 3: A study case, Memcheck(memory-leak detection tool on PCA).

<1> Compile Target program with clang & gold plugin

    refer: https://llvm.org/docs/GoldPlugin.html

<2> Run Memcheck in preload mode on the target program directory

    Memcheck -dir program_path_directory -pre 1
	
<3> Run Memcheck on the target program executable

    Memcheck -file program_executable
