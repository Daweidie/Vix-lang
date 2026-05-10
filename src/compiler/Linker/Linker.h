#ifndef VIX_LINKER_H
#define VIX_LINKER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* target_triple;
    int bare_mode;
    const char* linker_script;
    int static_link;
    const char* entry_point;
    const char* libc_dir;     /* directory containing bundled libc (CRT objects, DLLs, static libs) */
} VixLinkOptions;

int vix_link(const char* obj_file, const char* output_file,
             const VixLinkOptions* options, const char** error_msg);

#ifdef __cplusplus
}
#endif

#endif
