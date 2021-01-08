#pragma once

#include "firmware.h"
#include <utils/types.h>
#include <sec/tsec.h>
#include <sec/se.h>
#include <sec/se_t210.h>
#include "../types.h"

int keygen(u8 *keyblob, u32 kb, tsec_ctxt_t *tsec_ctxt, launch_ctxt_t *hos_ctxt);