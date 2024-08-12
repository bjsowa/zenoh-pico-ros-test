#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zenoh-pico.h"

#include "rmw/types.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/identifier.h"
#include "rosidl_typesupport_microxrcedds_c/message_type_support.h"
#include "std_msgs/msg/bool.h"
#include "ucdr/microcdr.h"

#define MAX_TOPIC_KEYEXPR_SIZE 250
#define CDR_ENCAPSULATION_SIZE 4

const uint8_t cdr_encapsulation[] = {0, 1, 0, 0};

std_msgs__msg__Bool my_bool;

bool create_topic_keyexpr_c_str(const size_t domain_id,
                                const char *const topic_name,
                                const char *const message_namespace,
                                const char *const message_name,
                                const char *const type_hash,
                                char *const buffer) {
  int size = snprintf(buffer, MAX_TOPIC_KEYEXPR_SIZE, "%d/%s/%s::dds_::%s_/%s",
                      domain_id, topic_name, message_namespace, message_name,
                      type_hash);
  if (size < 0 || size >= MAX_TOPIC_KEYEXPR_SIZE) {
    return false;
  }
  return true;
}

int main(int argc, char **argv) {

  std_msgs__msg__Bool__init(&my_bool);

  my_bool.data = true;

  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool);

  const rosidl_message_type_support_t *type_support_xrce =
      get_message_typesupport_handle(
          type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);

  const message_type_support_callbacks_t *type_support_callbacks =
      (const message_type_support_callbacks_t *)type_support_xrce->data;

  const rosidl_type_hash_t *type_hash =
      type_support->get_type_hash_func(type_support);
  // printf("Type hash: %s", type_hash->value);
  char *type_hash_c_str = NULL;
  rosidl_stringify_type_hash(type_hash, rcutils_get_default_allocator(),
                             &type_hash_c_str);

  uint8_t topic_keyexpr_c_str_buffer[MAX_TOPIC_KEYEXPR_SIZE];
  if (!create_topic_keyexpr_c_str(
          0, "talker", type_support_callbacks->message_namespace_,
          type_support_callbacks->message_name_, type_hash_c_str,
          topic_keyexpr_c_str_buffer)) {
    printf("Failed to create topic keyexpr\n");
    return -1;
  }

  printf("Topic keyexpr: %s\n", topic_keyexpr_c_str_buffer);

  ucdrBuffer ub;

  uint32_t serialized_size =
      CDR_ENCAPSULATION_SIZE +
      type_support_callbacks->get_serialized_size((const void *)&my_bool);

  uint8_t message_buffer[serialized_size];

  // printf("%d\n", type_support_callbacks->max_serialized_size());
  memcpy(message_buffer, cdr_encapsulation, CDR_ENCAPSULATION_SIZE);

  ucdr_init_buffer_origin_offset(&ub, message_buffer, serialized_size, 0,
                                 CDR_ENCAPSULATION_SIZE);

  type_support_callbacks->cdr_serialize((const void *)&my_bool, &ub);

  for (int i = 0; i < serialized_size; ++i) {
    printf("%u ", message_buffer[i]);
  }

  printf("\n");

  z_owned_config_t config = z_config_default();
  zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make("client"));
  zp_config_insert(z_loan(config), Z_CONFIG_CONNECT_KEY,
                   z_string_make("tcp/127.0.0.1:7447"));

  printf("Opening session...\n");
  z_owned_session_t s = z_open(z_move(config));
  if (!z_check(s)) {
    printf("Unable to open session!\n");
    return -1;
  }

  // Start read and lease tasks for zenoh-pico
  if (zp_start_read_task(z_loan(s), NULL) < 0 ||
      zp_start_lease_task(z_loan(s), NULL) < 0) {
    printf("Unable to start read and lease tasks\n");
    z_close(z_session_move(&s));
    return -1;
  }

  z_owned_publisher_t pub = z_declare_publisher(
      z_loan(s), z_keyexpr(topic_keyexpr_c_str_buffer), NULL);
  if (!z_check(pub)) {
    printf("Unable to declare publisher for key expression!\n");
    return -1;
  }

  size_t sequence_number = 0;
  uint8_t pub_gid[RMW_GID_STORAGE_SIZE] = {1, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

  z_bytes_t gid_bytes;
  gid_bytes.len = RMW_GID_STORAGE_SIZE;
  gid_bytes.start = pub_gid;

  z_owned_bytes_map_t map = z_bytes_map_new();
  z_bytes_map_insert_by_copy(&map, z_bytes_from_str("source_gid"), gid_bytes);
  z_bytes_map_insert_by_copy(&map, z_bytes_from_str("source_timestamp"),
                             z_bytes_from_str("1"));
  z_bytes_map_insert_by_copy(&map, z_bytes_from_str("sequence_number"),
                             z_bytes_from_str("1"));

  printf("Press CTRL-C to quit...\n");
  for (int idx = 0; idx < 100; ++idx) {
    sleep(1);

    z_publisher_put_options_t options = z_publisher_put_options_default();
    options.attachment = z_bytes_map_as_attachment(&map);
    z_publisher_put(z_loan(pub), (const uint8_t *)message_buffer,
                    serialized_size, &options);
  }

  z_undeclare_publisher(z_move(pub));

  // Stop read and lease tasks for zenoh-pico
  zp_stop_read_task(z_loan(s));
  zp_stop_lease_task(z_loan(s));

  z_close(z_move(s));

  return 0;
}
