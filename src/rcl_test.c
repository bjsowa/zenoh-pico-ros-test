#include <stdio.h>
#include <unistd.h>

#include "rcl/init.h"
#include "rcl/node.h"

#define RCCHECK(ret)                                                                         \
  {                                                                                          \
    if (((ret) != RCL_RET_OK)) {                                                             \
      printf("Failed status on line %d in file %s: %d. Aborting.", __LINE__, __FILE__, ret); \
    }                                                                                        \
  }

rcl_allocator_t allocator;
rcl_init_options_t init_options;
rcl_context_t context;
rcl_node_t node;

int main() {
  allocator = rcl_get_default_allocator();
  init_options = rcl_get_zero_initialized_init_options();
  context = rcl_get_zero_initialized_context();

  RCCHECK(rcl_init_options_init(&init_options, allocator));

  RCCHECK(rcl_init(0, NULL, &init_options, &context));

  return 0;
}