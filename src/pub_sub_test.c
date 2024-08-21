#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "geometry_msgs/msg/twist.h"
#include "rmw/types.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/identifier.h"
#include "rosidl_typesupport_microxrcedds_c/message_type_support.h"
#include "ucdr/microcdr.h"
#include "zenoh-pico.h"

#define MAX_TOPIC_KEYEXPR_SIZE 250
#define CDR_ENCAPSULATION_SIZE 4

#define PUB_TOPIC "pub_topic"
#define SUB_TOPIC "sub_topic"

#define ZENOH_MODE_KEY    "client"
#define ZENOH_CONNECT_KEY "tcp/127.0.0.1:7447"

const uint8_t cdr_encapsulation[] = {0, 1, 0, 0};

const rosidl_message_type_support_t *type_support;
const rosidl_message_type_support_t *type_support_xrce;
const message_type_support_callbacks_t *type_support_callbacks;
char *type_hash_c_str = NULL;

z_owned_config_t config;
z_owned_session_t session;

bool create_topic_keyexpr_c_str(const size_t domain_id, const char *const topic_name,
                                const char *const message_namespace, const char *const message_name,
                                const char *const type_hash, char *const buffer) {
  int size = snprintf(buffer, MAX_TOPIC_KEYEXPR_SIZE, "%d/%s/%s::dds_::%s_/%s", domain_id,
                      topic_name, message_namespace, message_name, type_hash);
  if (size < 0 || size >= MAX_TOPIC_KEYEXPR_SIZE) {
    return false;
  }
  return true;
}

void init_session() {
  z_config_new(&config);
  zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, ZENOH_MODE_KEY);
  zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, ZENOH_CONNECT_KEY);

  printf("Opening session...\n");

  if (z_open(&session, z_move(config)) < 0) {
    printf("Unable to open session!\n");
    exit(-1);
  }

  // Start read and lease tasks for zenoh-pico
  if (zp_start_read_task(z_loan_mut(session), NULL) < 0 ||
      zp_start_lease_task(z_loan_mut(session), NULL) < 0) {
    printf("Unable to start read and lease tasks\n");
    z_close(z_session_move(&session));
    exit(-1);
  }
}

void init_type_support() {
  type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist);

  type_support_xrce = get_message_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);

  type_support_callbacks = (const message_type_support_callbacks_t *)type_support_xrce->data;

  const rosidl_type_hash_t *type_hash = type_support->get_type_hash_func(type_support);
  rosidl_stringify_type_hash(type_hash, rcutils_get_default_allocator(), &type_hash_c_str);
}

// int8_t attachment_handler(z_bytes_t key, z_bytes_t value, void *ctx) {
//   printf(">>> %.*s: %.*s\n", (int)key.len, key.start, (int)value.len, value.start);
//   return 0;
// }

// void data_handler(const z_sample_t *sample, void *ctx) {
//   printf("Message Received\n");
//   if (z_attachment_check(&sample->attachment)) {
//     printf("Attachements:\n");
//     z_attachment_iterate(sample->attachment, attachment_handler, NULL);
//   }

//   printf("Payload:\n");
//   for (int i = 0; i < sample->payload.len; i++) {
//     printf("%x ", sample->payload.start[i]);
//   }
//   printf("\n");

//   ucdrBuffer ub;
//   geometry_msgs__msg__Twist twist;

//   ucdr_init_buffer(&ub, (void *)&sample->payload.start[CDR_ENCAPSULATION_SIZE],
//                    sample->payload.len - CDR_ENCAPSULATION_SIZE);

//   if (!type_support_callbacks->cdr_deserialize(&ub, (void *)&twist)) {
//     printf("Failed to deserialize message!");
//     return;
//   }

//   printf("Twist deserialized: (%.2f %.2f %.2f %.2f %.2f %.2f)\n", twist.linear.x, twist.linear.y,
//          twist.linear.z, twist.angular.x, twist.angular.y, twist.angular.z);
// }

// void declare_subscriber() {
//   uint8_t topic_keyexpr_c_str_buffer[MAX_TOPIC_KEYEXPR_SIZE];
//   if (!create_topic_keyexpr_c_str(0, SUB_TOPIC, type_support_callbacks->message_namespace_,
//                                   type_support_callbacks->message_name_, type_hash_c_str,
//                                   topic_keyexpr_c_str_buffer)) {
//     printf("Failed to create sub topic keyexpr\n");
//     exit(-1);
//   }

//   printf("Sub Topic keyexpr: %s\n", topic_keyexpr_c_str_buffer);

//   z_owned_closure_sample_t callback = z_closure(data_handler);
//   z_owned_subscriber_t sub = z_declare_subscriber(
//       z_loan(session), z_keyexpr(topic_keyexpr_c_str_buffer), z_move(callback), NULL);
//   if (!z_check(sub)) {
//     printf("Unable to declare subscriber.\n");
//     exit(-1);
//   }
// }

// void publish_messages() {
//   uint8_t topic_keyexpr_c_str_buffer[MAX_TOPIC_KEYEXPR_SIZE];
//   if (!create_topic_keyexpr_c_str(0, PUB_TOPIC, type_support_callbacks->message_namespace_,
//                                   type_support_callbacks->message_name_, type_hash_c_str,
//                                   topic_keyexpr_c_str_buffer)) {
//     printf("Failed to create pub topic keyexpr\n");
//     exit(-1);
//   }

//   printf("Pub Topic keyexpr: %s\n", topic_keyexpr_c_str_buffer);

//   ucdrBuffer ub;
//   geometry_msgs__msg__Twist twist;

//   memset(&twist, 0, sizeof(twist));

//   geometry_msgs__msg__Twist__init(&twist);

//   uint32_t serialized_size = type_support_callbacks->get_serialized_size((const void *)&twist);

//   uint8_t message_buffer[CDR_ENCAPSULATION_SIZE + serialized_size];
//   memcpy(message_buffer, cdr_encapsulation, CDR_ENCAPSULATION_SIZE);

//   ucdr_init_buffer(&ub, &message_buffer[CDR_ENCAPSULATION_SIZE], serialized_size);

//   z_owned_publisher_t pub =
//       z_declare_publisher(z_loan(session), z_keyexpr(topic_keyexpr_c_str_buffer), NULL);
//   if (!z_check(pub)) {
//     printf("Unable to declare publisher for key expression!\n");
//     exit(-1);
//   }

//   size_t sequence_number = 0;
//   uint8_t pub_gid[RMW_GID_STORAGE_SIZE];
//   z_random_fill(pub_gid, RMW_GID_STORAGE_SIZE);

//   z_bytes_t gid_bytes;
//   gid_bytes.len = RMW_GID_STORAGE_SIZE;
//   gid_bytes.start = pub_gid;

//   z_owned_bytes_map_t map = z_bytes_map_new();
//   z_bytes_map_insert_by_copy(&map, z_bytes_from_str("source_gid"), gid_bytes);
//   // TODO
//   z_bytes_map_insert_by_copy(&map, z_bytes_from_str("source_timestamp"), z_bytes_from_str("1"));
//   // TODO
//   z_bytes_map_insert_by_copy(&map, z_bytes_from_str("sequence_number"), z_bytes_from_str("1"));

//   z_publisher_put_options_t options = z_publisher_put_options_default();
//   options.attachment = z_bytes_map_as_attachment(&map);

//   while (true) {
//     sleep(1);

//     ++sequence_number;
//     printf("Publishing message %d\n", sequence_number);

//     twist.linear.x += 1.0;

//     ucdr_reset_buffer(&ub);
//     type_support_callbacks->cdr_serialize((const void *)&twist, &ub);

//     z_publisher_put(z_loan(pub), (const uint8_t *)message_buffer,
//                     CDR_ENCAPSULATION_SIZE + serialized_size, &options);
//   }
// }

int main(int argc, char **argv) {
  init_session();
  init_type_support();
  // declare_subscriber();
  // publish_messages();

  return 0;
}
