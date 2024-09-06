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
      rcutils_error_string_t error_str = rcutils_get_error_string();                           \
      printf("%s\n", error_str.str);                                                           \
      exit(-1);                                                                                \
    }                                                                                          \
  }

#define RCSOFTCHECK(RCL_COMMAND)                                                                 \
  {                                                                                              \
    rcl_ret_t ret = RCL_COMMAND;                                                                 \
    if (ret != RCL_RET_OK) {                                                                     \
      printf("Failed status on line %d in file %s: %d. Continuing.\n", __LINE__, __FILE__, ret); \
      rcutils_error_string_t error_str = rcutils_get_error_string();                             \
      printf("%s\n", error_str.str);                                                             \
    }                                                                                            \
  }

rcl_allocator_t allocator;
rcl_init_options_t init_options;
rclc_support_t support;
rcl_node_t node;
rcl_node_options_t node_options;
rclc_executor_t executor;
rcl_publisher_t publisher;
rcl_subscription_t subscription;
rcl_client_t client;
rcl_timer_t timer;
rcl_service_t service;

geometry_msgs__msg__Twist sub_twist_msg;
example_interfaces__srv__AddTwoInts_Response client_add_two_ints_res;
example_interfaces__srv__AddTwoInts_Request service_add_two_ints_req;
example_interfaces__srv__AddTwoInts_Response service_add_two_ints_res;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  printf("Timer triggered\n");

  printf("Publishing twist\n");
  geometry_msgs__msg__Twist twist_msg;
  geometry_msgs__msg__Twist__init(&twist_msg);
  twist_msg.linear.y = 5.0;
  RCSOFTCHECK(rcl_publish(&publisher, &twist_msg, NULL))

  printf("Sending service request\n");
  example_interfaces__srv__AddTwoInts_Request add_two_ints_req;
  example_interfaces__srv__AddTwoInts_Request__init(&add_two_ints_req);
  int64_t seq;
  add_two_ints_req.a = 5;
  add_two_ints_req.b = 10;
  RCSOFTCHECK(rcl_send_request(&client, &add_two_ints_req, &seq))
}

void sub_callback(const void *msgin) {
  const geometry_msgs__msg__Twist *msg = msgin;
  printf("Received twist message: %f %f %f\n", msg->linear.x, msg->linear.y, msg->linear.z);
}

void client_callback(const void *resin) {
  const example_interfaces__srv__AddTwoInts_Response *res = resin;
  printf("Received service response: %d\n", res->sum);
}

void service_callback(const void *reqin, void *resin) {
  const example_interfaces__srv__AddTwoInts_Request *req = reqin;
  example_interfaces__srv__AddTwoInts_Response *res = resin;
  printf("Received service request: %d + %d = ?\n", req->a, req->b);
  res->sum = req->a + req->b;
}

int main() {
  allocator = rcl_get_default_allocator();
  init_options = rcl_get_zero_initialized_init_options();
  node = rcl_get_zero_initialized_node();
  node_options = rcl_node_get_default_options();
  subscription = rcl_get_zero_initialized_subscription();
  client = rcl_get_zero_initialized_client();
  executor = rclc_executor_get_zero_initialized_executor();
  timer = rcl_get_zero_initialized_timer();
  service = rcl_get_zero_initialized_service();

  geometry_msgs__msg__Twist__init(&sub_twist_msg);
  example_interfaces__srv__AddTwoInts_Request__init(&service_add_two_ints_req);
  example_interfaces__srv__AddTwoInts_Response__init(&service_add_two_ints_res);
  example_interfaces__srv__AddTwoInts_Response__init(&client_add_two_ints_res);

  // Init options
  RCCHECK(rcl_init_options_init(&init_options, allocator))

  // Support
  RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator))

  // Node
  RCCHECK(rclc_node_init_with_options(&node, "rcl_test", "", &support, &node_options))

  // Executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 10, &allocator))

  // Publisher
  RCCHECK(rclc_publisher_init_best_effort(
      &publisher, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "ping"))

  // Subscription
  RCCHECK(rclc_subscription_init_default(
      &subscription, &node, ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "~/sub"));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscription, &sub_twist_msg, sub_callback,
                                         ON_NEW_DATA))

  // Client
  RCCHECK(rclc_client_init_default(&client, &node,
                                   ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, srv, AddTwoInts),
                                   "/add_two_ints"));
  RCCHECK(rclc_executor_add_client(&executor, &client, &client_add_two_ints_res, client_callback))

  // Service
  RCCHECK(rclc_service_init_default(
      &service, &node, ROSIDL_GET_SRV_TYPE_SUPPORT(example_interfaces, srv, AddTwoInts),
      "/add_two_ints"));
  RCCHECK(rclc_executor_add_service(&executor, &service, &service_add_two_ints_req,
                                    &service_add_two_ints_res, service_callback))

  // Timer
  RCCHECK(rclc_timer_init_default2(&timer, &support, RCL_MS_TO_NS(1500), timer_callback, true));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // Preallocate executor
  RCCHECK(rclc_executor_prepare(&executor))

  rclc_executor_spin(&executor);

  return 0;
}