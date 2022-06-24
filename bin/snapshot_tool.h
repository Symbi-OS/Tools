#ifndef SNAPSHOT_TOOL_H
#define SNAPSHOT_TOOL_H

uint64_t phys_to_virt(uint64_t phys_addr){
  uint64_t phys = (phys_addr >> 12) << 12;
  uint64_t virt = 0xffff888000000000;
  return (phys + virt);
}

#endif
