/* librist. Copyright 2019-2020 SipRadius LLC. All right reserved.
 * Author: Kuldeep Singh Dhaka <kuldeep@madresistor.com>
 * Author: Sergio Ammirata, Ph.D. <sergio@ammirata.net>
 */

#ifndef LIBRIST_H
#define LIBRIST_H

#include <stdint.h>
#include <stdlib.h>

/* __BEGIN_DECLS should be used at the beginning of your declarations,
   so that C++ compilers don't mangle their names.  Use __END_DECLS at
   the end of C declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/* Reference: http://gcc.gnu.org/wiki/Visibility */
#if defined(_WIN32) || defined(__CYGWIN__)
# if defined(rist_EXPORTS)
#  if defined(__GNUC__)
#   define RIST_API __attribute__ ((dllexport))
#  else /* defined(__GNUC__) */
			/* Note: actually gcc seems to also supports this syntax. */
#   define RIST_API __declspec(dllexport)
#  endif /* defined(__GNUC__) */
# else /* defined(rist_EXPORTS) */
#  if defined(__GNUC__)
#   define RIST_API __attribute__ ((dllimport))
#  else
			/* Note: actually gcc seems to also supports this syntax. */
#   define RIST_API __declspec(dllimport)
#  endif
# endif /* defined(rist_EXPORTS) */
#else /* defined(_WIN32) || defined(__CYGWIN__) */
	#if __GNUC__ >= 4
		#define RIST_API __attribute__ ((visibility ("default")))
# else /* __GNUC__ >= 4 */
		#define RIST_API
# endif /* __GNUC__ >= 4 */
#endif /* defined(_WIN32) || defined(__CYGWIN__) */

__BEGIN_DECLS

/* Used for cname, miface and shared secret */
#define RIST_MAX_STRING_SHORT 128
/* Used for url/address */
#define RIST_MAX_STRING_LONG 256

/* Track PROTOCOL and API changes */
#define RIST_PROTOCOL_VERSION (2)
#define RIST_API_VERSION (8)
#define RIST_SUBVERSION (2)
#define RIST_PEER_CONFIG_VERSION (0)

/* Default peer config values */
#define RIST_DEFAULT_VIRT_SRC_PORT (1971)
#define RIST_DEFAULT_VIRT_DST_PORT (1968)
#define RIST_DEFAULT_RECOVERY_MODE RIST_RECOVERY_MODE_TIME
#define RIST_DEFAULT_RECOVERY_MAXBITRATE (100000)
#define RIST_DEFAULT_RECOVERY_MAXBITRATE_RETURN (0)
#define RIST_DEFAULT_RECOVERY_LENGHT_MIN (1000)
#define RIST_DEFAULT_RECOVERY_LENGHT_MAX (1000)
#define RIST_DEFAULT_RECOVERY_REORDER_BUFFER (25)
#define RIST_DEFAULT_RECOVERY_RTT_MIN (50)
#define RIST_DEFAULT_RECOVERY_RTT_MAX (500)
#define RIST_DEFAULT_BUFFER_BLOAT_MODE RIST_BUFFER_BLOAT_MODE_OFF
#define RIST_DEFAULT_BUFFER_BLOAT_LIMIT	(6)
#define RIST_DEFAULT_BUFFER_BLOAT_HARD_LIMIT (20)
#define RIST_DEFAULT_VERBOSE_LEVEL RIST_LOG_WARN
#define RIST_DEFAULT_PROFILE RIST_PROFILE_MAIN
#define RIST_DEFAULT_SESSION_TIMEOUT (60000)
#define RIST_DEFAULT_KEEPALIVE_INTERVAL (1000)

/* Rist URL parameter names for peer config */
#define RIST_URL_PARAM_BUFFER_SIZE     "buffer"
#define RIST_URL_PARAM_SECRET          "secret"
#define RIST_URL_PARAM_AES_TYPE        "aes-type"
#define RIST_URL_PARAM_BANDWIDTH       "bandwidth"
#define RIST_URL_PARAM_RET_BANDWIDTH   "return-bandwidth"
#define RIST_URL_PARAM_REORDER_BUFFER  "reorder-buffer"
#define RIST_URL_PARAM_RTT             "rtt"
#define RIST_URL_PARAM_COMPRESSION     "compression"
#define RIST_URL_PARAM_CNAME           "cname"
#define RIST_URL_PARAM_VIRT_DST_PORT   "virt-dst-port"
#define RIST_URL_PARAM_WEIGHT          "weight"
#define RIST_URL_PARAM_MIFACE          "miface"
#define RIST_URL_PARAM_SESSION_TIMEOUT "session-timeout"
#define RIST_URL_PARAM_KEEPALIVE_INT   "keepalive-interval"
/* Less common URL parameters */
#define RIST_URL_PARAM_BUFFER_SIZE_MIN  "buffer-min"
#define RIST_URL_PARAM_BUFFER_SIZE_MAX  "buffer-max"
#define RIST_URL_PARAM_RTT_MIN          "rtt-min"
#define RIST_URL_PARAM_RTT_MAX          "rtt-max"
#define RIST_URL_PARAM_AES_KEY_ROTATION "key-rotation"
#define RIST_URL_PARAM_BUFFER_BLOAT_MODE       "buffer-bloat-mode"
#define RIST_URL_PARAM_BUFFER_BLOAT_LIMIT      "buffer-bloat-limit"
#define RIST_URL_PARAM_BUFFER_BLOAT_HARD_LIMIT "buffer-bloat-hard-limit"
/* Rist additional parameter names */
#define RIST_URL_PARAM_VIRT_SRC_PORT   "virt-src-port"
#define RIST_URL_PARAM_PROFILE         "profile"
#define RIST_URL_PARAM_VERBOSE_LEVEL   "verbose-level"

enum rist_nack_type {
	RIST_NACK_RANGE = 0,
	RIST_NACK_BITMASK = 1,
};

enum rist_profile {
	RIST_PROFILE_SIMPLE = 0,
	RIST_PROFILE_MAIN = 1,
	RIST_PROFILE_ADVANCED = 2,
};

enum rist_log_level {
	RIST_LOG_QUIET = -1,
	RIST_LOG_INFO = 0,
	RIST_LOG_ERROR = 1,
	RIST_LOG_WARN = 2,
	RIST_LOG_DEBUG = 3,
	RIST_LOG_SIMULATE = 4,
};

enum rist_recovery_mode {
	RIST_RECOVERY_MODE_UNCONFIGURED = 0,
	RIST_RECOVERY_MODE_DISABLED = 1,
	RIST_RECOVERY_MODE_BYTES = 2,
	RIST_RECOVERY_MODE_TIME = 3,
};

enum rist_buffer_bloat_mode {
	RIST_BUFFER_BLOAT_MODE_OFF = 0,
	RIST_BUFFER_BLOAT_MODE_NORMAL = 1,
	RIST_BUFFER_BLOAT_MODE_AGGRESSIVE = 2
};

enum rist_data_block_flags {
	RIST_DATA_FLAGS_USE_SEQ = 1,
	RIST_DATA_FLAGS_NEED_FREE = 2,
};

enum rist_stats_type {
	RIST_STATS_SENDER_PEER,
	RIST_STATS_RECEIVER_FLOW
};

struct rist_receiver;
struct rist_sender;
struct rist_peer;

struct rist_data_block {
	const void *payload;
	size_t payload_len;
	uint64_t ts_ntp;
	/* The virtual source and destination ports are not used for simple profile */
	uint16_t virt_src_port;
	/* These next fields are not needed/used by rist_sender_data_write */
	uint16_t virt_dst_port;
	struct rist_peer *peer;
	uint32_t flow_id;
	/* Get's populated by librist with the rtp_seq on output, can be used on input to tell librist which rtp_seq to use */
	uint64_t seq;
	uint32_t flags;
};

struct rist_oob_block {
	struct rist_peer *peer;
	const void *payload;
	size_t payload_len;
	uint64_t ts_ntp;
};

struct rist_peer_config {
	int version;

	/* Communication parameters */
	// If a value of 0 is specified for address family, the library 
	// will parse the address and populate all communication parameters.
	// Alternatively, use either AF_INET or AF_INET6 and address will be
	// treated like an IP address or hostname
	int address_family; 
	int initiate_conn;
	char address[RIST_MAX_STRING_LONG];
	char miface[RIST_MAX_STRING_SHORT];
	uint16_t physical_port;

	/* The virtual destination port is not used for simple profile */
	uint16_t virt_dst_port;

	/* Recovery options */
	enum rist_recovery_mode recovery_mode;
	uint32_t recovery_maxbitrate;
	uint32_t recovery_maxbitrate_return;
	uint32_t recovery_length_min;
	uint32_t recovery_length_max;
	uint32_t recovery_reorder_buffer;
	uint32_t recovery_rtt_min;
	uint32_t recovery_rtt_max;
	
	/* Load balancing weight (use 0 for duplication) */
	uint32_t weight;

	/* Encryption */
	char secret[RIST_MAX_STRING_SHORT];
	int key_size;
	uint32_t key_rotation;

	/* Compression (sender only as receiver is auto detect) */
	int compression;

	/* cname identifier for rtcp packets */
	char cname[RIST_MAX_STRING_SHORT];

	/* Congestion control */
	enum rist_buffer_bloat_mode buffer_bloat_mode;
	uint32_t buffer_bloat_limit;
	uint32_t buffer_bloat_hard_limit;

	/* Connection options */
	uint32_t session_timeout;
	uint32_t keepalive_interval;

};

struct rist_stats_sender_peer {
	/* cname */
	char cname[RIST_MAX_STRING_SHORT];
	/* internal peer id */
	uint32_t peer_id;
	/* avg bandwidth calculation */
	size_t bandwidth;
	/* bandwidth devoted to retries */
	size_t retry_bandwidth;
	/* num sent packets  (TODO: excluding retries?!?) */
	uint64_t sent;
	/* num received packets TODO ???*/
	uint64_t received;
	/* retransmitted packets */
	uint64_t retransmitted;
	/* packets skipped due to bufferbloat protection */
	uint32_t bloat_skipped;
	/* retransmits skipped */
	uint32_t retransmit_skipped;
	/* quality: Q = (sent * 100.0) / sent + bloat_skipped + retransmit_skipped + retransmitted */
	double quality;
	/* current RTT */
	uint32_t rtt;
	/* avg rtt last 8 calculations */
	uint32_t avg_rtt;
	/* TODO ??? */
	size_t retry_buffer_size;
	/* TODO ??? */
	uint32_t cooldown_time;
};

struct rist_stats_receiver_flow_peer {
	/* cname */
	char cname[RIST_MAX_STRING_SHORT];

	uint32_t flow_id;
	/* dead peer 0/1 */
	uint32_t dead;

	/* peer num within flow peer list */
	uint32_t peer_num;
	uint32_t flow_peer_list_len;

	/* peer id */
	uint32_t peer_id;
	/* received packets */
	uint32_t received;
	/* missing, including reordered */
	uint32_t missing;
	/* Q = (received * 100.0) / received + missing */
	double quality;
	/* total recovered packets */
	uint32_t recovered_total;
	/* recovered without nacks sent (reordered) */
	uint32_t recovered_no_nack;
	/* recovered after N nacks */
	uint32_t recovered_one_nack;
	uint32_t recovered_two_nacks;
	uint32_t recovered_three_nacks;
	uint32_t recovered_more_nacks;
	/* ??? TODO */
	uint32_t recovered_average;
	uint32_t recovered_slope;
	uint32_t recovered_slope_inverse;
	/* reordered packets */
	uint32_t reordered;
	/* duplicate packets received */
	uint32_t duplicates;
	/* retries ??? TODO */
	uint32_t retries;
	uint64_t recovery_buffer_length;
	/* missing queue */
	uint32_t missing_queue;
	uint32_t missing_queue_max;
	/* current rtt */
	uint32_t rtt;
	/* avg rtt over last 8 stat intervals */
	uint32_t avg_rtt;
	/* bitrate */
	uint32_t bitrate;
	/* average bitrate */
	uint32_t avg_bitrate;
};

struct rist_stats_receiver_flow {
	uint32_t flow_id;
	uint64_t received;
	/* missing, including reordered */
	uint32_t missing;
	/* Q = (received * 100.0) / received + missing */
	double quality;
	/* total recovered packets */
	uint32_t recovered_total;
	/* recovered without nacks sent (reordered) */
	uint32_t recovered_no_nack;
	/* recovered after N nacks */
	uint32_t recovered_one_nack;
	uint32_t recovered_two_nacks;
	uint32_t recovered_three_nacks;
	uint32_t recovered_more_nacks;
	/* lost packets */
	uint32_t lost;
	/* reordered packets */
	uint32_t reordered;
	/* duplicate packets received */
	uint32_t duplicates;
	/* retries ??? TODO */
	uint32_t retries;
	/* ??? */
	uint64_t min_inter_packet_spacing;
	uint64_t cur_inter_packet_spacing;
	uint64_t max_inter_packet_spacing;

	uint32_t peer_list_len;

	struct rist_stats_receiver_flow_peer peers[1];
};

struct rist_stats {
	enum rist_stats_type stats_type;
	union {
		struct rist_stats_sender_peer rist_stats_sender_peer;
		struct rist_stats_receiver_flow rist_stats_receiver_flow;
	} stats;
};

/**
 * @brief Create Sender
 *
 * Create a RIST sender instance
 *
 * @param[out] ctx a context representing the sender instance
 * @param profile RIST profile
 * @param flow_id Flow ID, use 0 to delegate creation of flow_id to lib
 * @param log_level Level of log messages to display
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_sender_create(struct rist_sender **ctx, enum rist_profile profile,
				uint32_t flow_id, enum rist_log_level log_level);

/**
 * @brief Assign dynamic authentication handler
 *
 * Whenever a new peer is connected, @a connect_cb is called.
 * Whenever a new peer is disconnected, @a disconn_cb is called.
 *
 * @param ctx RIST sender context
 * @param connect_cb A pointer to the function that will be called when a new peer
 * connects. Return 0 or -1 to authorize or decline (NULL function pointer is valid)
 * @param disconn_cb A pointer to the function that will be called when a new peer
 * is marked as dead (NULL function pointer is valid)
 * @param arg is an the extra argument passed to the `conn_cb` and `disconn_cb`
 */
RIST_API int rist_sender_auth_handler_set(struct rist_sender *ctx,
		int (*connect_cb)(void *arg, const char* conn_ip, uint16_t conn_port, const char* local_ip, uint16_t local_port, struct rist_peer *peer),
		int (*disconn_cb)(void *arg, struct rist_peer *peer),
		void *arg);

/**
 * @brief Add a peer connector to the existing sender.
 *
 * One sender can send data to multiple peers.
 *
 * @param ctx RIST sender context
 * @param[out] peer Store the new peer pointer
 * @param config a pointer to the struct rist_peer_config, which contains
 *        the configuration parameters for the peer endpoint.
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_sender_peer_create(struct rist_sender *ctx,
		struct rist_peer **peer, const struct rist_peer_config *config);

/**
 * @brief Remove a peer connector to the existing sender.
 *
 * @param ctx RIST sender context
 * @param peer a pointer to the struct rist_peer, which
 *        points to the peer endpoint.
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_sender_peer_destroy(struct rist_sender *ctx,
		struct rist_peer *peer);

/**
 * @brief Set RIST max jitter
 *
 * Set max jitter
 *
 * @param ctx RIST sender context
 * @param t max jitter in ms
 * @return 0 on success, -1 on error
 */
RIST_API int rist_sender_jitter_max_set(struct rist_sender *ctx, int t);

/**
 * @brief Enable out-of-band data channel
 *
 * Call after receiver initialization to enable out-of-band data.
 *
 * @param ctx RIST sender context
 * @param oob_callback A pointer to the function that will be called when out-of-band data
 * comes in (NULL function pointer is valid)
 * @param arg is an the extra argument passed to the `oob_callback`
 * @return 0 on success, -1 on error
 */
RIST_API int rist_sender_oob_callback_set(struct rist_sender *ctx, 
		int (*oob_callback)(void *arg, const struct rist_oob_block *oob_block),
		void *arg);

/**
 * @brief Kickstart a pre-configured sender
 *
 * After all the peers have been added, this function triggers
 * the sender to start
 *
 * @param ctx RIST sender context
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_sender_start(struct rist_sender *ctx);

/**
 * @brief Write data directly to a remote receiver peer.
 *
 * This API is used to transmit out-of-band data to a remote receiver peer
 *
 * @param ctx RIST sender context
 * @param oob_block a pointer to the struct rist_oob_block
 * @return number of written bytes on success, -1 in case of error.
 */
RIST_API int rist_sender_oob_write(struct rist_sender *ctx, const struct rist_oob_block *oob_block);

/**
 * @brief Reads out-of-band data
 *
 * Use this API to read out-of-band data from an internal fifo queue instead of the callback
 *
 * @param ctx RIST sender context
 * @param[out] oob_block pointer to the rist_oob_block structure
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_sender_oob_read(struct rist_sender *ctx, const struct rist_oob_block **oob_block);

/**
 * @brief Write data into a librist packet.
 *
 * One sender can send write data into a librist packet.
 *
 * @param ctx RIST sender context
 * @param data_block pointer to the rist_data_block structure
 * the ts_ntp will be populated by the lib if a value of 0 is passed
 * @return number of written bytes on success, -1 in case of error.
 */
RIST_API int rist_sender_data_write(struct rist_sender *ctx, const struct rist_data_block *data_block);

/**
 * @brief Destroy RIST sender
 *
 * Destroy the RIST instance
 *
 * @param ctx RIST sender context
 * @return 0 on success, -1 on error
 */
RIST_API int rist_sender_destroy(struct rist_sender *ctx);

/**
 * @brief Retrieve the current flow_id value
 *
 * Retrieve the current flow_id value
 *
 * @param ctx RIST sender context
 * @param flow_id pointer to your flow_id variable
 * @return 0 on success, -1 on error
 */
RIST_API int rist_sender_flow_id_get(struct rist_sender *ctx, uint32_t *flow_id);

/**
 * @brief Change the flow_id value
 *
 * Change the flow_id value
 *
 * @param ctx RIST sender context
 * @param flow_id new flow_id
 * @return 0 on success, -1 on error
 */
RIST_API int rist_sender_flow_id_set(struct rist_sender *ctx, uint32_t flow_id);

/**
 * Create a RIST receiver instance
 *
 * @param[out] ctx a context representing the receiver instance
 * @param profile RIST profile
 * @param log_level Level of log messages to display
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_create(struct rist_receiver **ctx, enum rist_profile profile,
			enum rist_log_level log_level);

/**
 * @brief Assign dynamic authentiation handler
 *
 * Whenever a new peer is connected, @a conn_cb is called.
 * Whenever a new peer is disconnected, @a disconn_cb is called.
 *
 * @param ctx RIST sender context
 * @param connect_cb A pointer to the function that will be called when a new peer
 * connects. Return 0 or -1 to authorize or decline (NULL function pointer is valid)
 * @param disconn_cb A pointer to the function that will be called when a new peer
 * is marked as dead (NULL function pointer is valid)
 * @param arg is an the extra argument passed to the `conn_cb` and `disconn_cb`
 */
RIST_API int rist_receiver_auth_handler_set(struct rist_receiver *ctx,
		int (*connect_cb)(void *arg, const char* conn_ip, uint16_t conn_port, const char* local_ip, uint16_t local_port, struct rist_peer *peer),
		int (*disconn_cb)(void *arg, struct rist_peer *peer),
		void *arg);

/**
 * @brief Add a peer connector to the existing receiver.
 *
 * One receiver can receive data from multiple peers.
 *
 * @param ctx RIST sender context
 * @param[out] peer Store the new peer pointer
 * @param config a pointer to the struct rist_peer_config, which contains
 *        the configuration parameters for the peer endpoint.
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_peer_create(struct rist_receiver *ctx, 
		struct rist_peer **peer, const struct rist_peer_config *config);

/**
 * @brief Remove a peer connector to the existing receiver.
 *
 * @param ctx RIST receiver context
 * @param peer a pointer to the struct rist_peer, which
 *        points to the peer endpoint.
 * @return 0 on success, -1 in case of error.
 */
RIST_API int rist_receiver_peer_destroy(struct rist_receiver *ctx,
		struct rist_peer *peer);

/**
 * @brief Set RIST max jitter
 *
 * Set max jittter
 *
 * @param ctx RIST receiver context
 * @param t max jitter in ms
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_jitter_max_set(struct rist_receiver *ctx, int t);

/**
 * @brief Enable out-of-band data channel
 *
 * Call after receiver initialization to enable out-of-band data.
 *
 * @param ctx RIST receiver context
 * @param oob_callback A pointer to the function that will be called when out-of-band data
 * comes in (NULL function pointer is valid)
 * @param arg is an the extra argument passed to the `oob_callback`
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_oob_callback_set(struct rist_receiver *ctx, 
		int (*oob_callback)(void *arg, const struct rist_oob_block *oob_block),
		void *arg);

/**
 * @brief Configure nack type
 *
 * Choose the nack type used by the receiver.
 *
 * @param ctx RIST receiver context
 * @param nack_type 0 for range (default), 1 for bitmask
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_nack_type_set(struct rist_receiver *ctx, enum rist_nack_type nacks_type);

/**
 * @brief Enable data callback channel
 *
 * Call to enable data callback channel.
 *
 * @param ctx RIST receiver context
 * @param data_callback The function that will be called when a data frame is
 * received from a sender.
 * @param arg the extra argument passed to the `data_callback`
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_data_callback_set(struct rist_receiver *ctx,
	int (*data_callback)(void *arg, const struct rist_data_block *data_block),
	void *arg);

/**
 * @brief Setup receiver start
 *
 * Start receiver data output thread.
 *
 * @param ctx RIST receiver context
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_start(struct rist_receiver *ctx);

/**
 * @brief Write data directly to a remote sender peer.
 *
 * This API is used to transmit out-of-band data to a remote sender peer
 *
 * @param ctx RIST receiver context
 * @param oob_block a pointer to the struct rist_oob_block
 * @return number of written bytes on success, -1 on error
 */
RIST_API int rist_receiver_oob_write(struct rist_receiver *ctx, const struct rist_oob_block *oob_block);

/**
 * @brief Reads out-of-band data
 *
 * Use this API to read out-of-band data from an internal fifo queue instead of the callback
 *
 * @param ctx RIST receiver context
 * @param[out] oob_block a pointer to the rist_oob_block structure
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_oob_read(struct rist_receiver *ctx, const struct rist_oob_block **oob_block);

/**
 * @brief Reads rist data
 *
 * Use this API to read data from an internal fifo queue instead of the callback
 *
 * @param ctx RIST receiver context
 * @param[out] data_block a pointer to the rist_data_block structure
 * @param timeout How long to wait for queue data (ms), 0 for no wait
 * @return num buffers remaining on queue +1 (0 if no buffer returned), -1 on error
 */
RIST_API int rist_receiver_data_read(struct rist_receiver *ctx, const struct rist_data_block **data_block, int timeout);

/**
 * @brief Destroy RIST receiver
 *
 * Destroy RIST receiver instance
 *
 * @param ctx RIST receiver context
 * @return 0 on success, -1 on error
 */
RIST_API int rist_receiver_destroy(struct rist_receiver *ctx);

/**
 * @brief Parses url for peer config data (encryption, compression, etc)
 *
 * Use this API to parse a generic URL string and turn it into a meaninful peer_config structure
 *
 * @param url a pointer to a url to be parsed, i.e. rist://myserver.net:1234?buffer=100&cname=hello
 * @param[out] peer_config a pointer to a the rist_peer_config structure (NULL is allowed).
 * When passing NULL, the library will allocate a new rist_peer_config structure with the latest
 * default values and it expects the application to free it when it is done using it.
 * @return 0 on success or non-zero on error. The value returned is actually the number
 * of parameters that are valid
 */
RIST_API int rist_parse_address(const char *url, const struct rist_peer_config **peer_config);

/**
 * @brief Set custom file descriptor and target ip:port to be used for printing logs
 *
 * Set fd or configure remote udp address:port to print librist logs
 *
 * @param fd file descriptor to be used for statistics (default is no logs or -1, 2 is stderr)
 * @param address ipaddress:port combination to send the logs using udp (NULL to disable)
 *
 * @return 0 on success, -1 on error
 */
RIST_API int rist_logs_set(int fd, char *address);

/**
 * @brief Set callback for receiving stats structs
 *
 * @param ctx RIST Receiver context
 * @param statsinterval interval between stats reporting
 * @param stats_cb Callback function that will be called. The stats pointer must be free()'d when you are finished.
 * @param arg extra arguments for callback function
 */
RIST_API int rist_receiver_stats_callback_set(struct rist_receiver *ctx, int statsinterval, int (*stats_cb)(void *arg, struct rist_stats *stats), void *arg);

/**
 * @brief Set callback for receiving stats structs
 *
 * @param ctx RIST Sender context
 * @param statsinterval interval between stats reporting
 * @param stats_cb Callback function that will be called. The stats pointer must be free()'d when you are finished.
 * @param arg extra arguments for callback function
 */
RIST_API int rist_sender_stats_callback_set(struct rist_sender *ctx, int statsinterval, int (*stats_cb)(void *arg, struct rist_stats *stats), void *arg);
__END_DECLS

#endif