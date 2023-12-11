#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>
using namespace llvm;

static LLVMContext Context;
static Module *ModuleOb = new Module("my compiler", Context);
static std::vector<std::string> FunArgs;

Function *createFunc(IRBuilder<> &Builder, std::string Name) {
    std::vector<Type *> Integers(FunArgs.size(), Type::getInt32Ty(Context));
    FunctionType *funcType = llvm::FunctionType::get(Builder.getInt32Ty(), Integers, false);
    Function *fooFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, Name, ModuleOb);
    return fooFunc;
}

void setFuncArgs(Function *fooFunc, std::vector<std::string> FunArgs) {
    unsigned Idx = 0;
    for (auto &Arg : fooFunc->args())
        Arg.setName(FunArgs[Idx++]);
}

BasicBlock *createBB(Function *fooFunc, std::string Name) {
    return BasicBlock::Create(Context, Name, fooFunc);
}

Value *createArith(IRBuilder<> &Builder, Value *L, Value *R) {
    return Builder.CreateMul(L, R, "multmp");
}

int main(int argc, char *argv[]) {
    FunArgs.push_back("a");
    FunArgs.push_back("b");
    static IRBuilder<> Builder(Context);
    Function *fooFunc = createFunc(Builder, "foo");
    setFuncArgs(fooFunc, FunArgs);
    BasicBlock *entry = createBB(fooFunc, "entry");
    Builder.SetInsertPoint(entry);
    Function::arg_iterator AI = fooFunc->arg_begin();
    Value *Arg1 = AI++;
    Value *Arg2 = AI;
    Value *constant = Builder.getInt32(16);
    Value *val = createArith(Builder, Arg1, constant);

    Value *StartVal = Builder.getInt32(1);
    Value *EndVal = Arg2;
    BasicBlock *LoopBB = createBB(fooFunc, "loop");
    BasicBlock *AfterLoop = createBB(fooFunc, "afterloop");
    Builder.CreateBr(LoopBB);
    Builder.SetInsertPoint(LoopBB);

    PHINode *phi = Builder.CreatePHI(Type::getInt32Ty(Context), 2, "i");
    phi->addIncoming(StartVal, entry);
    Value *Add = Builder.CreateAdd(val, Builder.getInt32(5), "addtmp");
    Value *Next = Builder.CreateAdd(phi, Builder.getInt32(1), "next");
    Value *EndCond = Builder.CreateICmpSLT(phi, EndVal, "endcond");
    Builder.CreateCondBr(EndCond, LoopBB, AfterLoop);
    phi->addIncoming(Next, LoopBB);

    Builder.SetInsertPoint(AfterLoop);
    Builder.CreateRet(Add);

    verifyFunction(*fooFunc);
    ModuleOb->print(llvm::errs(), nullptr);

    return 0;
}

