#include <llvm/IR/IRBuilder.h>
namespace sysio { namespace chain { namespace sysvmoc {
namespace LLVMJIT {

llvm::Value* CreateInBoundsGEPWAR(llvm::IRBuilder<>& irBuilder, llvm::Value* Ptr, llvm::Value* v1, llvm::Value* v2) {
   if(!v2)
      return irBuilder.CreateInBoundsGEP(Ptr, v1);
   else
      return irBuilder.CreateInBoundsGEP(Ptr, {v1, v2});
}

}

}}}