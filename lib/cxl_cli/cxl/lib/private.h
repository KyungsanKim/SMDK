/* SPDX-License-Identifier: LGPL-2.1 */
/* Copyright (C) 2020-2021, Intel Corporation. All rights reserved. */
#ifndef _LIBCXL_PRIVATE_H_
#define _LIBCXL_PRIVATE_H_

#include <libkmod.h>
#include <libudev.h>
#include <cxl/cxl_mem.h>
#include <ccan/endian/endian.h>
#include <ccan/short_types/short_types.h>
#include <util/size.h>

#define CXL_EXPORT __attribute__ ((visibility("default")))

struct cxl_pmem {
	int id;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
};

struct cxl_endpoint;
struct cxl_memdev {
	int id, major, minor;
	int numa_node;
	void *dev_buf;
	size_t buf_len;
	char *host_path;
	char *dev_path;
	char *firmware_version;
	struct cxl_ctx *ctx;
	struct list_node list;
	unsigned long long pmem_size;
	unsigned long long ram_size;
	int payload_max;
	size_t lsa_size;
	struct kmod_module *module;
	struct cxl_pmem *pmem;
	unsigned long long serial;
	struct cxl_endpoint *endpoint;
};

struct cxl_dport {
	int id;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
	char *phys_path;
	char *fw_path;
	struct cxl_port *port;
	struct list_node list;
};

enum cxl_port_type {
	CXL_PORT_ROOT,
	CXL_PORT_SWITCH,
	CXL_PORT_ENDPOINT,
};

struct cxl_port {
	int id;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
	char *uport;
	int ports_init;
	int endpoints_init;
	int decoders_init;
	int dports_init;
	int nr_dports;
	int depth;
	struct cxl_ctx *ctx;
	struct cxl_bus *bus;
	enum cxl_port_type type;
	struct cxl_port *parent;
	struct kmod_module *module;
	struct list_node list;
	struct list_head child_ports;
	struct list_head endpoints;
	struct list_head decoders;
	struct list_head dports;
};

struct cxl_bus {
	struct cxl_port port;
};

struct cxl_endpoint {
	struct cxl_port port;
	struct cxl_memdev *memdev;
};

struct cxl_target {
	struct list_node list;
	struct cxl_decoder *decoder;
	char *dev_path;
	char *phys_path;
	char *fw_path;
	int id, position;
};

struct cxl_decoder {
	struct cxl_port *port;
	struct list_node list;
	struct cxl_ctx *ctx;
	u64 start;
	u64 size;
	u64 dpa_resource;
	u64 dpa_size;
	u64 max_available_extent;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
	int nr_targets;
	int id;
	enum cxl_decoder_mode mode;
	unsigned int interleave_ways;
	unsigned int interleave_granularity;
	bool pmem_capable;
	bool volatile_capable;
	bool mem_capable;
	bool accelmem_capable;
	bool locked;
	enum cxl_decoder_target_type target_type;
	int regions_init;
	struct list_head targets;
	struct list_head regions;
	struct list_head stale_regions;
};

enum cxl_decode_state {
	CXL_DECODE_UNKNOWN = -1,
	CXL_DECODE_RESET = 0,
	CXL_DECODE_COMMIT,
};

struct cxl_region {
	struct cxl_decoder *decoder;
	struct list_node list;
	int mappings_init;
	struct cxl_ctx *ctx;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
	int id;
	uuid_t uuid;
	u64 start;
	u64 size;
	unsigned int interleave_ways;
	unsigned int interleave_granularity;
	enum cxl_decode_state decode_state;
	struct kmod_module *module;
	struct list_head mappings;
};

struct cxl_memdev_mapping {
	struct cxl_region *region;
	struct cxl_decoder *decoder;
	unsigned int position;
	struct list_node list;
};

enum cxl_cmd_query_status {
	CXL_CMD_QUERY_NOT_RUN = 0,
	CXL_CMD_QUERY_OK,
	CXL_CMD_QUERY_UNSUPPORTED,
};

/**
 * struct cxl_cmd - CXL memdev command
 * @memdev: the memory device to which the command is being sent
 * @query_cmd: structure for the Linux 'Query commands' ioctl
 * @send_cmd: structure for the Linux 'Send command' ioctl
 * @input_payload: buffer for input payload managed by libcxl
 * @output_payload: buffer for output payload managed by libcxl
 * @refcount: reference for passing command buffer around
 * @query_status: status from query_commands
 * @query_idx: index of 'this' command in the query_commands array
 * @status: command return status from the device
 */
struct cxl_cmd {
	struct cxl_memdev *memdev;
	struct cxl_mem_query_commands *query_cmd;
	struct cxl_send_command *send_cmd;
	void *input_payload;
	void *output_payload;
	int refcount;
	int query_status;
	int query_idx;
	int status;
};

#define CXL_CMD_IDENTIFY_FW_REV_LENGTH 0x10

struct cxl_cmd_identify {
	char fw_revision[CXL_CMD_IDENTIFY_FW_REV_LENGTH];
	le64 total_capacity;
	le64 volatile_capacity;
	le64 persistent_capacity;
	le64 partition_align;
	le16 info_event_log_size;
	le16 warning_event_log_size;
	le16 failure_event_log_size;
	le16 fatal_event_log_size;
	le32 lsa_size;
	u8 poison_list_max_mer[3];
	le16 inject_poison_limit;
	u8 poison_caps;
	u8 qos_telemetry_caps;
} __attribute__((packed));

struct cxl_cmd_get_lsa_in {
	le32 offset;
	le32 length;
} __attribute__((packed));

struct cxl_cmd_set_lsa {
	le32 offset;
	le32 rsvd;
	unsigned char lsa_data[0];
} __attribute__((packed));

/**************** smdk ****************/

struct media_error_record {
	le64 dpa;
	le32 len;
	le32 rsvd;
} __attribute__((packed));

struct cxl_cmd_poison_get_list_in {
	le64 address;
	le64 address_length;
} __attribute__((packed));

struct cxl_cmd_poison_get_list {
	u8 flags;
	u8 rsvd;
	le64 overflow_timestamp;
	le16 count;
	u8 rsvd2[0x14];
	struct media_error_record rcd[];
} __attribute__((packed));

struct cxl_cmd_clear_poison_in {
	le64 address;
	unsigned int clear_data[16];
} __attribute__((packed));

struct cxl_cmd_clear_event_record_in {
	u8 event_type;
	u8 flags;
	u8 n_event_handle;
	u8 rsvd[3];
	le16 event_record_handle;
} __attribute__((packed));

struct common_event_record {
	unsigned short event_record_len : 8;
	unsigned short event_record_flags : 6;
	unsigned int flag_rsvd : 18;
	le32 event_record_handle;
	le64 timestamp;
	u8 rsvd[0x10];
} __attribute__((packed));

struct media_event {
	unsigned int uuid[4];
	struct common_event_record rcd;
	le64 physical_address;
	u8 memory_event_desc;
	u8 memory_event_type;
	u8 transaction_type;
	le16 validity_flags;
	u8 channel;
	u8 rank;
	int device : 24;
	le64 component_identifier[2];
	u8 rsvd[0x2e];
} __attribute__((packed));

struct cxl_cmd_get_event_record_out {
	u8 flags;
	u8 rsvd;
	le16 overflow_count;
	le64 first_overflow_timestamp;
	le64 last_overflow_timestamp;
	le16 event_records_count;
	u8 rsvd2[10];
	struct media_event event[];
} __attribute__((packed));
#define POISON_ADDR_MASK 0xFFFFFFFFFFFFFFF8
#define POISON_SOURCE_MASK 0x7
#ifndef BIT
#define BIT(_x) (1 << (_x))
#endif

/* CXL 3.0 8.2.9.8.1.1 Identify Memory Device Poison Handling Capabilities */
#define CXL_CMD_IDENTIFY_POISON_HANDLING_CAPABILITIES_INJECTS_PERSISTENT_POISON_MASK \
	BIT(0)
#define CXL_CMD_IDENTIFY_POISON_HANDLING_CAPABILITIES_SCANS_FOR_POISON_MASK \
	BIT(1)

/* CXL 3.0 8.2.9.8.1.1 Identify Memory Device QoS Telemetry Capabilities */
#define CXL_CMD_IDENTIFY_QOS_TELEMETRY_CAPABILITIES_EGRESS_PORT_CONGESTION_MASK \
	BIT(0)
#define CXL_CMD_IDENTIFY_QOS_TELEMETRY_CAPABILITIES_TEMPORARY_THROUGHPUT_REDUCTION_MASK \
	BIT(1)

/* CXL 3.0. 8.2.9.8.3.3 Set Alert Configuration */
struct cxl_cmd_set_alert_config {
	u8 valid_alert_actions;
	u8 enable_alert_actions;
	u8 life_used_prog_warn_threshold;
	u8 rsvd;
	le16 dev_over_temperature_prog_warn_threshold;
	le16 dev_under_temperature_prog_warn_threshold;
	le16 corrected_volatile_mem_err_prog_warn_threshold;
	le16 corrected_pmem_err_prog_warn_threshold;
} __attribute__((packed));

/* CXL 3.0 8.2.9.3.1 Get FW Info */
#define CXL_CMD_FW_INFO_FW_REV_LENGTH 0x10

struct cxl_cmd_get_firmware_info {
	u8 slots_supported;
	u8 slot_info;
	u8 activation_caps;
	u8 rsvd[13];
	char fw_revisions[4][CXL_CMD_FW_INFO_FW_REV_LENGTH];
} __attribute__((packed));

/* CXL 3.0 8.2.9.3.1 Get FW Info Byte 1 FW Slot Info */
#define CXL_CMD_FW_INFO_SLOT_ACTIVE_MASK GENMASK(2, 0)
#define CXL_CMD_FW_INFO_SLOT_STAGED_MASK GENMASK(5, 3)

/* CXL 3.0 8.2.9.3.1 Get FW Info Byte 2 FW Activation Capabilities */
#define CXL_CMD_FW_INFO_ACTIVATION_CAPABILITIES_ONLINE_FW_ACTIVATION_MASK BIT(0)

/* CXL 3.0 8.2.9.3.2 Transfer FW */
struct cxl_cmd_transfer_firmware {
	u8 action;
	u8 slot;
	le16 rsvd;
	le32 offset;
	u8 rsvd2[0x78];
	unsigned char fw_data[0];
} __attribute__((packed));

/* CXL 3.0 8.2.9.3.3 Activate FW */
struct cxl_cmd_activate_firmware {
	u8 action;
	u8 slot;
} __attribute__((packed));
/**************** smdk ****************/

struct cxl_cmd_get_health_info {
	u8 health_status;
	u8 media_status;
	u8 ext_status;
	u8 life_used;
	le16 temperature;
	le32 dirty_shutdowns;
	le32 volatile_errors;
	le32 pmem_errors;
} __attribute__((packed));
/* CXL 3.0 8.2.9.8.3.2 Get Alert Configuration */
struct cxl_cmd_get_alert_config {
	u8 valid_alerts;
	u8 programmable_alerts;
	u8 life_used_crit_alert_threshold;
	u8 life_used_prog_warn_threshold;
	le16 dev_over_temperature_crit_alert_threshold;
	le16 dev_under_temperature_crit_alert_threshold;
	le16 dev_over_temperature_prog_warn_threshold;
	le16 dev_under_temperature_prog_warn_threshold;
	le16 corrected_volatile_mem_err_prog_warn_threshold;
	le16 corrected_pmem_err_prog_warn_threshold;
} __attribute__((packed));

/* CXL 3.0 8.2.9.8.3.2 Get Alert Configuration Byte 0 Valid Alerts */
#define CXL_CMD_ALERT_CONFIG_VALID_ALERTS_LIFE_USED_PROG_WARN_THRESHOLD_MASK   \
	BIT(0)
#define CXL_CMD_ALERT_CONFIG_VALID_ALERTS_DEV_OVER_TEMPERATURE_PROG_WARN_THRESHOLD_MASK \
	BIT(1)
#define CXL_CMD_ALERT_CONFIG_VALID_ALERTS_DEV_UNDER_TEMPERATURE_PROG_WARN_THRESHOLD_MASK \
	BIT(2)
#define CXL_CMD_ALERT_CONFIG_VALID_ALERTS_CORRECTED_VOLATILE_MEM_ERR_PROG_WARN_THRESHOLD_MASK \
	BIT(3)
#define CXL_CMD_ALERT_CONFIG_VALID_ALERTS_CORRECTED_PMEM_ERR_PROG_WARN_THRESHOLD_MASK \
	BIT(4)

/* CXL 3.0 8.2.9.8.3.2 Get Alert Configuration Byte 1 Programmable Alerts */
#define CXL_CMD_ALERT_CONFIG_PROG_ALERTS_LIFE_USED_PROG_WARN_THRESHOLD_MASK    \
	BIT(0)
#define CXL_CMD_ALERT_CONFIG_PROG_ALERTS_DEV_OVER_TEMPERATURE_PROG_WARN_THRESHOLD_MASK \
	BIT(1)
#define CXL_CMD_ALERT_CONFIG_PROG_ALERTS_DEV_UNDER_TEMPERATURE_PROG_WARN_THRESHOLD_MASK \
	BIT(2)
#define CXL_CMD_ALERT_CONFIG_PROG_ALERTS_CORRECTED_VOLATILE_MEM_ERR_PROG_WARN_THRESHOLD_MASK \
	BIT(3)
#define CXL_CMD_ALERT_CONFIG_PROG_ALERTS_CORRECTED_PMEM_ERR_PROG_WARN_THRESHOLD_MASK \
	BIT(4)

struct cxl_cmd_get_partition {
	le64 active_volatile;
	le64 active_persistent;
	le64 next_volatile;
	le64 next_persistent;
} __attribute__((packed));

#define CXL_CAPACITY_MULTIPLIER		SZ_256M

struct cxl_cmd_set_partition {
	le64 volatile_size;
	u8 flags;
} __attribute__((packed));

/* CXL 2.0 8.2.9.5.2 Set Partition Info */
#define CXL_CMD_SET_PARTITION_FLAG_IMMEDIATE				BIT(0)

/* CXL 2.0 8.2.9.5.3 Byte 0 Health Status */
#define CXL_CMD_HEALTH_INFO_STATUS_MAINTENANCE_NEEDED_MASK		BIT(0)
#define CXL_CMD_HEALTH_INFO_STATUS_PERFORMANCE_DEGRADED_MASK		BIT(1)
#define CXL_CMD_HEALTH_INFO_STATUS_HW_REPLACEMENT_NEEDED_MASK		BIT(2)

/* CXL 2.0 8.2.9.5.3 Byte 1 Media Status */
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_NORMAL				0x0
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_NOT_READY			0x1
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_PERSISTENCE_LOST		0x2
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_DATA_LOST			0x3
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_POWERLOSS_PERSISTENCE_LOSS	0x4
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_SHUTDOWN_PERSISTENCE_LOSS	0x5
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_PERSISTENCE_LOSS_IMMINENT	0x6
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_POWERLOSS_DATA_LOSS		0x7
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_SHUTDOWN_DATA_LOSS		0x8
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_DATA_LOSS_IMMINENT		0x9

/* CXL 2.0 8.2.9.5.3 Byte 2 Additional Status */
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_MASK				GENMASK(1, 0)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_NORMAL			(0)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_WARNING			(1)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_CRITICAL			(2)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_MASK			GENMASK(3, 2)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_NORMAL			(0)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_WARNING			(1)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_CRITICAL			(2)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_MASK			BIT(4)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_NORMAL		(0)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_WARNING		(1)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_MASK		BIT(5)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_NORMAL		(0)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_WARNING		(1)

#define CXL_CMD_HEALTH_INFO_LIFE_USED_NOT_IMPL				0xff
#define CXL_CMD_HEALTH_INFO_TEMPERATURE_NOT_IMPL			0xffff

static inline int check_kmod(struct kmod_ctx *kmod_ctx)
{
	return kmod_ctx ? 0 : -ENXIO;
}

#endif /* _LIBCXL_PRIVATE_H_ */
