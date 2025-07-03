#include "vbe.h"

bool VBE_info_has_been_set = false;

struct VBEInfo VBE_info;
struct VBEModeInfo VBE_mode_info;

void VBE_setup_infos(const struct VBEInfo *const src_info,
		     const struct VBEModeInfo *src_mode_info)
{
	VBE_info = *src_info;
	VBE_mode_info = *src_mode_info;
	VBE_info_has_been_set = true;
}
