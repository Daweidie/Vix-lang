#ifndef VIX_LLC_H
#define VIX_LLC_H

#ifdef __cplusplus

#include <string>
#include <llvm/Support/CodeGen.h>

class Llc {
public:
	static bool compileToObject(const std::string &llvm_ir_path,
								const std::string &out_path,
								const std::string &triple,
								bool staticReloc,
								std::string &errMsg);

	static bool compileToAssembly(const std::string &llvm_ir_path,
								  const std::string &out_path,
								  const std::string &triple,
								  bool staticReloc,
								  std::string &errMsg);

private:
	static bool compile(const std::string &llvm_ir_path,
						const std::string &out_path,
						const std::string &triple,
						bool staticReloc,
						llvm::CodeGenFileType fileType,
						std::string &errMsg);
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int llc_compile_to_object(const char *llvm_ir_path,
						  const char *out_path,
						  const char *triple,
						  int staticReloc,
						  const char **errMsg);

int llc_compile_to_assembly(const char *llvm_ir_path,
							const char *out_path,
							const char *triple,
							int staticReloc,
							const char **errMsg);

#ifdef __cplusplus
}
#endif

#endif
