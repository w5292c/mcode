#ifndef MCODE_CMD_ENGINE_H
#define MCODE_CMD_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

void cmd_engine_init (void);
void cmd_engine_deinit (void);

void cmd_engine_start (void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_CMD_ENGINE_H */
