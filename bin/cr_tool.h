#ifndef __CR_TOOL__
#define __CR_TOOL__

struct params{
  bool get_cr4_flag;
  bool set_cr4_flag;
  bool modify_nosmep_flag;
  bool modify_nosmap_flag;
  bool disable_smap_smep;
  bool enable_smap_smep;

  uint64_t set_val;
};
#endif
