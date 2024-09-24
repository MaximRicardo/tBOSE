

#include "vbe.h"

struct VBE_Info VBE_info;
struct VBE_ModeInfo VBE_mode_info;

void VBE_setup_infos(const struct VBE_Info *const src_info, const struct VBE_ModeInfo *src_mode_info) {
    VBE_info = *src_info;
    VBE_mode_info = *src_mode_info;
}
