#include <stdio.h>
#include <unistd.h>

#include "rclc/executor.h"
#include "rclc/rclc.h"
// #include "std_msgs/msg/string.h"
#include "example_interfaces/srv/add_two_ints.h"
#include "geometry_msgs/msg/twist.h"

#define RCCHECK(RCL_COMMAND)                                                                   \
  {                                                                                            \
    rcl_ret_t ret = RCL_COMMAND;                                                               \
    if (ret != RCL_RET_OK) {                                                                   \
      printf("Failed status on line %d in file %s: %d. Aborting.\n", __LINE__, __FILE__, ret); \
      exit(-1);                                                                                \
    }                                                                                          \
  }

rcl_allocator_t allocator;
rcl_init_options_t init_options;
rclc_support_t support;
rcl_node_t node;
rcl_node_options_t node_options;
// rclc_executor_t executor;
rcl_publisher_t ping_publisher;
geometry_msgs__msg__Twist twist_msg;
rcl_subscription_t twist_sub;
rcl_client_t add_two_ints_client;
// rcl_timer_t ping_timer;
// rcl_wait_set_t wait_set;

void ping_timer_callback(rcl_timer_t* timer, int64_t last_call_time) {
  printf("ping\n");
  // rcl_ret_t rc;
  // RCLC_UNUSED(last_call_time);
  // if (timer != NULL) {
  //   //printf("Timer: time since last call %d\n", (int) last_call_time);
  //   // rc = rcl_publish(&ping_publisher, &pingNode_ping_msg, NULL);
  //   if (rc == RCL_RET_OK) {
  //     printf("Published message %s\n", pingNode_ping_msg.data.data);
  //   } else {
  //     printf("timer_callback: Error publishing message %s\n", pingNode_ping_msg.data.data);
  //   }
  // } else {
  //   printf("timer_callback Error: timer parameter is NULL\n");
  // }
}

int main() {
  allocator = rcl_get_default_allocator();
  init_options = rcl_get_zero_initialized_init_options();
  node = rcl_get_zero_initialized_node();
  node_options = rcl_node_get_default_options();
  twist_sub = rcl_get_zero_initialized_subscription();
  add_two_ints_client = rcl_get_zero_initialized_client();
  // executor = rclc_executor_get_zero_initialized_executor();
  // ping_timer = rcl_get_zero_initialized_timer();
  // wait_set = rcl_get_zero_initialized_wait_set();

  RCCHECK(rcl_init_options_init(&init_options, allocator))
  RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator))
  RCCHECK(rclc_node_init_with_options(&node, "rcl_test", "", &support, &node_options))

  printf("Node initialized!\n");

  RCCHECK(rclc_publisher_init_best_effort(
      &ping_publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "ping"))

  RCCHECK(rclc_subscription_init_default(
      &twist_sub, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "~/sub"));

  RCCHECK(rclc_client_init_default(&add_two_ints_client, &node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, srv, AddTwoInts),
                                   "/add_two_ints"));

  geometry_msgs__msg__Twist__init(&twist_msg);
  twist_msg.linear.x = 20.0;

  while (true) {
    RCCHECK(rcl_publish(&ping_publisher, &twist_msg, NULL))
    sleep(1);
    if (rcl_take(&twist_sub, &twist_msg, NULL, NULL) != RMW_RET_OK) {
      rcutils_error_string_t error_str = rcutils_get_error_string();
      rcutils_reset_error();
      printf("Failed to take message: %s\n", error_str.str);
    }
    sleep(1);
  }

  // RCCHECK(rclc_executor_init(&executor, &support.context, 10, &allocator))

  // RCCHECK(rclc_timer_init_default2(&ping_timer, &support, RCL_MS_TO_NS(1000),
  // ping_timer_callback, true));

  // RCCHECK(rcl_wait_set_init(&wait_set, 0, 0, 1, 0, 0, 0, &support.context, allocator))
  // RCCHECK(rclc_executor_add_timer(&executor, &ping_timer))

  // RCCHECK(rclc_executor_prepare(&executor))

  // rclc_executor_spin(&executor);

  // rcl_timer_call(&)

  return 0;
}