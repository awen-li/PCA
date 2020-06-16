#Title
PCA: Memory Leak Detection using Partial Call-Path Analysis.

# Introduction
Essentially, *PCA* is a well implemented data-dependence analyzer targeting c program, in which we applied our optimization such as 
partial call-path analysis and integer encoding in reaching-definition computation to improve efficiency and effectiveness.
*PCA-Mem* is a case study for evaluation, our tool also supports many other data-dependence based applications such as taint analysis, bug detection and so on.

# Installation
Here we present two ways to fetch and use PCA:

## UsePCAthrough Virtual Machine
-Step 1. Download PCA-VM.zip (the virtual disk image) through the [link](https://drive.google.com/file/d/12eMHiYnqYPwjgpd6BjKtmmiT73y9lGC4/view?usp=sharing).
-Step 2. Launch the virtual machine in Virtual Box(username/password: pca/pca, root/pca).
-Step 3. Open a terminal, and go to the directory/home/pca/PCA.

## UsePCAthrough source code compilation.
-Step 1. Download source code through the [link](https://github.com/Daybreak2019/PCA).
-Step 2. Entry directory PCA/llvm7 and run installLLVM.sh, which will install LLVM7 and configure environmentvariables automatically.
-Step 3. Entry directory PCA and build PCA with script build.sh.

# Usage
we implemented a case study of PCA: PCA-Mem. This part shows how to run PCA-Mem against target program for memory leak detection.

## Check prerequisites
we guarantee that {\tool} can be installed and run successfully with the support of following prerequisites:
\begin{itemize}
\item UNIX          (Ubuntu 16.04 LTS or Ubuntu 18.04 LTS)
\item LLVM          (v7.0.0)
\end{itemize}

## Compile target program
To enable data-dependence analysis based on LLVM, target program needs to be compiled with clang and gold-plugin (details referred to [here](https://llvm.org/docs/GoldPlugin.html}{\color{blue)). Usually we need to specify the compiler as clang and set compile parameters as following:
-Compile Flag: -emit-llvm
-Link Flag:    -ftlo

## Run PCA-Mem against simple program and debugging
In this step, we present how to do memory leak detection with PCA-Mem and generate data-dependence graph (DDG) for graphical display.

A small test case (leak.bc) is shown as Figure~\ref{fig:case1}, there are two partial free defects obviously, which are detected by PCA-Mem (with command: PCA-Mem -file leak.bc) as expected (Figure~\ref{fig:case1_res}) 

we then present a parameter (--dump-DDG) to generate DDG.dot, which can be open by GVEdit as below (ICFG is painted with black color while DDG with red), in this case we could verify the correctness of DDG manually while debugging.

## Run PCA-Mem against Slurm
For large scale program like Slurm (Version 15.08.7), which usually contains multiple modules, PCA-Mem conducts two-step analysis:
-Pre-process: Find dependencies between modules (e.g., PCA-Mem -dir Slurm -pre=1). 
-Program analysis: Link all necessary IR of modules for the target executable and Perform data dependence analysis and memory leak detection sequentially (e.g., PCA-Mem -file Slurm/salloc.bc). 

        

