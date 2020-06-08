

#include <llvm/Support/CommandLine.h>	
#include <llvm/IR/InstIterator.h>
#include "Oaf.h"
#include "common/Stat.h"


using namespace llvm;


bool Oaf::runOnModule(ModuleManage& ModMng) 
{
    Stat::StartTime ("Compute Data dependence");

    DgGraph *gGraph = new DgGraph (ModMng);

    Stat::EndTime ("Compute Data dependence");

    return AF_FALSE;
}



