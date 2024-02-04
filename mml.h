#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void mmlInit(void);
void play(const char *mml);
void play3(const char *mml1, const char *mml2, const char *mml3);
void play3n(const char *mml1, const char *mml2, const char *mml3, const char *mml_noise);
void mmlBeginCompile(u8 *p, u16 len);
u16 mmlEndCompile(void);

#ifdef __cplusplus
};
#endif
