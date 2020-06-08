

#include <llvm-c/Core.h>  
#include <llvm/IR/Module.h>
#include <llvm/Support/CommandLine.h>	 
#include <llvm/Support/FileSystem.h>	 
#include <llvm/Bitcode/BitcodeWriterPass.h>  
#include <llvm/IR/LegacyPassManager.h>		 
#include <llvm/Support/Signals.h>	 
#include <llvm/IRReader/IRReader.h>	 
#include <llvm/Support/ToolOutputFile.h>  
#include <llvm/Support/PrettyStackTrace.h>  
#include <llvm/IR/LLVMContext.h>		 
#include <llvm/Support/SourceMgr.h>  
#include <llvm/Bitcode/BitcodeWriterPass.h>
#include <llvm/IR/DataLayout.h>
#include "llvm/IR/InstVisitor.h"
#include "llvmadpt/ModuleSet.h"
#include "common/VisitDir.h"
#include "llvmadpt/LlvmAdpt.h"
#include "analysis/points-to/Anderson.h"
#include "analysis/Dependence.h"


using namespace llvm;
using namespace std;


static cl::opt<std::string>
InputDirectory("dir", cl::desc("<input bitcode file directory>"), cl::value_desc("directory"));

static cl::opt<std::string>
InputFilename("file", cl::desc("<input bitcode file>"), cl::init("-"), cl::value_desc("filename"));

VOID GetModulePath (vector<string> &ModulePathVec, int argc, char ** argv)
{
    cl::ParseCommandLineOptions(argc, argv, "call graph analysis\n");
    
    if (InputFilename == "" || InputDirectory == "-")
    {
        errs() << "InputFilename is NULL \n ";
        return;
    }

    if (InputFilename != "-" && InputFilename != "")
    {
        std::string ModuleName = InputFilename;
        ModulePathVec.push_back (ModuleName);
    }
    else
    {  
        std::string ModuleDir = InputDirectory;       
        ModulePath ModulePt;        
        ModulePt.Visit(ModuleDir.c_str(), &ModulePathVec);
    }

    return;
}


inline BOOL CheckPointers (llvm::Value *Val)
{
    std::vector<llvm::Value*> PtsTo;
    llvmAdpt::GetPtsTo (Val, PtsTo);

    for (auto it = PtsTo.begin(); it != PtsTo.end(); it++)
    {
        llvm::Value *PtsVal = *it;
        if (PtsVal == NULL)
        {
            return AF_TRUE;
        }
    }

    return AF_FALSE;
}

VOID DumpInst (llvm::Instruction *Inst)
{
    errs() << "ERROR: reading from a null pointer at ";
    if(Inst->getDebugLoc()) 
    {
        //If clang is run with -g flag, print line number
        errs()<<"line number:" <<Inst->getDebugLoc().getLine()<< ", col:" << Inst->getDebugLoc().getCol()<<"\n";
    }
    else 
    {
        //otherwise, just display the address location of the error
        errs()<< Inst<<"\n";
    }
                
    //Inst->dump();
    errs()<<*Inst<<Inst->getName()<<"\n";

    return;
}


VOID AnalyzePts (vector<string> &ModulePathVec)
{
    ModuleManage ModuleMng (ModulePathVec);
    PointsTo PtsTo (ModuleMng, T_ANDRESEN);

    for (auto it = ModuleMng.func_begin (); it != ModuleMng.func_end (); it++)
    {
        llvm::Function *Func = *it;
        
        /* Visit all load/store instructions */
        for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
        {
            llvm::Instruction *Inst = &*itr.getInstructionIterator();

            if (Inst->getOpcode() == Instruction::Load)
            {
                if (!llvmAdpt::IsValuePtrType (Inst)) 
                {     
                    continue;
                }

                if (CheckPointers (Inst))
                {
                    DumpInst(Inst);
                }
            }
            else if (Inst->getOpcode() == Instruction::Store)
            {
                if (llvmAdpt::IsValuePtrType (Inst->getOperand(0)))
                {
                    continue;
                }

                if (CheckPointers (Inst->getOperand(1)))
                {
                    DumpInst(Inst);
                }
            }
            else
            {
            }
        }
    }
    

    return;
}

int main(int argc, char ** argv) 
{ 
    vector<string> ModulePathVec;
    
    PassRegistry &Registry = *PassRegistry::getPassRegistry();

    initializeCore(Registry);
    initializeScalarOpts(Registry);
    initializeIPO(Registry);
    initializeAnalysis(Registry);
    initializeTransformUtils(Registry);
    initializeInstCombine(Registry);
    initializeInstrumentation(Registry);
    initializeTarget(Registry);
  
    GetModulePath(ModulePathVec, argc, argv);
    if (ModulePathVec.size() == 0)
    {
        errs()<<"get none module paths!!\n";
        return 0;
    }

    AnalyzePts(ModulePathVec);

    return 0;
}

