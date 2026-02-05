#ifndef QBE_OPT_H
#define QBE_OPT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Optimize a QBE-style SSA file in-place. */
void qbe_opt_file(const char *filename);

#ifdef __cplusplus
}
#endif

#endif // QBE_OPT_H
