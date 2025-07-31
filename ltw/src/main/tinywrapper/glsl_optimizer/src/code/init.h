//
// Created by hanji on 2024/10/20.
//

#ifndef FOLD_CRAFT_LAUNCHER_INIT_H
#define FOLD_CRAFT_LAUNCHER_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

void MesaConverterInit();
void *open_lib(const char **names, const char *override);

#ifdef __cplusplus
} /* extern C */
#endif


#endif //FOLD_CRAFT_LAUNCHER_INIT_H
