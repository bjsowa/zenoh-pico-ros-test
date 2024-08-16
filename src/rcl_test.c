#include <stdio.h>
#include <unistd.h>

#include "rcl/init.h"
#include "rcl/node.h"

#define RCCHECK(ret)                                                                           \
  {                                                                                            \
    if (((ret) != RCL_RET_OK)) {                                                               \
      const rcutils_error_state_t* error_state = rcutils_get_error_state();                    \
      printf("Failed status on line %d in file %s: %d. Aborting.\n", __LINE__, __FILE__, ret); \
      printf("%s\n", error_state->message);                                                    \
    }                                                                                          \
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