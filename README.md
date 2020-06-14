# PCA

README: PCA: Memory Leak Detection using Partial Call-Path Analysis

================================================
Use PCA with Virtual Machine
================================================
1> Download Virtual Disk Image for PCA from https://drive.google.com/file/d/12eMHiYnqYPwjgpd6BjKtmmiT73y9lGC4/view?usp=sharing
2> Open the Image with VirtualBox
3> Account Information: pca/pca, root/pca
4> Use PCA in the directory /home/pca/PCA

================================================
Source compilation and installation
================================================
1> Compile LLVM7.
cd PCA/llvm7
sudo ./installLLVM.sh
2> Compile PCA and application tools based on it.
cd PCA
./build.sh
3> A study case, PcaMen(memory-leak detection tool on PCA).
   3.1> Compile Target program with clang & gold plugin
        refer: https://llvm.org/docs/GoldPlugin.html
   3.2> Run PcaMen in preload mode on the target program directory
        PcaMem -dir program_path_directory -pre 1	
   3.3> Run PcaMen on the target program executable
        PcaMem -file program_executable

