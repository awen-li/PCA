# hpcanalysis

README: OAF_An optimized data flow analyzer based on LLVM

Step 1: Compile LLVM 7.0 with golden plugin
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


Step 2: Compile OAF and application tools based on it
<1> Configure environment
	#The following three configuration could be added into "/etc/profile"
	export LLVM_PATH=/your_path/llvm7.0.0
	export CLANG_PATH=$LLVM_PATH/build/bin
	export PATH=$PATH:$CLANG_PATH

<2> build OAF
	cd /your_path/oaf
	./build.sh
	#the executable of tool would be generated to build/bin


Step 3: A study case, Memcheck(memory-leak detection tool on OAF)
<1> Compile Target program with clang & gold plugin
    refer: https://llvm.org/docs/GoldPlugin.html

<2> Run Memcheck in preload mode on the target program directory
    Memcheck -dir program_path_directory -pre 1
	
<3> Run Memcheck on the target program executable
    Memcheck -file program_executable
