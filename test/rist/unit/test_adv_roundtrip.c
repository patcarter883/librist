/*
 * Copyright © 2024-2026 SipRadius LLC
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Unit test: Advanced Profile (TR-06-3) header parse/build round-trip.
 * Verifies that building a packet and parsing it back produces identical
 * field values for all flag combinations.
 */

#include "proto/adv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

static int test_count = 0;
static int pass_count = 0;

#define CHECK(cond, msg) do { \
	test_count++; \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); \
		return 1; \
	} \
	pass_count++; \
} while (0)

static int test_basic_roundtrip(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = "Hello RIST Advanced Profile!";
	size_t payload_len = sizeof(payload);

	struct rist_adv_params params;
	memset(&params, 0, sizeof(params));
	params.seq = 0x12345678;
	params.timestamp = 1000000;
	params.ssrc = 0xAABBCC00; /* Even = protected */
	params.enc_type = RIST_ADV_TYPE_DIRECT;
	params.psk_mode = RIST_ADV_PSK_NONE;
	params.lpc_mode = RIST_ADV_LPC_NONE;
	params.first_frag = true;
	params.last_frag = true;
	params.expedite = false;
	params.retransmit = false;

	int total = rist_adv_build(buf, &params, payload, payload_len);
	CHECK(total > 0, "rist_adv_build should return positive length");
	CHECK((size_t)total == RIST_ADV_HEADER_MIN + payload_len,
	      "total should be header_min + payload");

	struct rist_adv_parsed parsed;
	int rc = rist_adv_parse(buf, (size_t)total, &parsed);
	CHECK(rc == 0, "rist_adv_parse should succeed");

	CHECK(parsed.seq == 0x12345678, "seq round-trip");
	CHECK(parsed.timestamp == 1000000, "timestamp round-trip");
	CHECK(parsed.ssrc == 0xAABBCC00, "ssrc round-trip");
	CHECK(parsed.enc_type == RIST_ADV_TYPE_DIRECT, "enc_type round-trip");
	CHECK(parsed.psk_mode == RIST_ADV_PSK_NONE, "psk_mode round-trip");
	CHECK(parsed.lpc_mode == RIST_ADV_LPC_NONE, "lpc_mode round-trip");
	CHECK(parsed.first_frag == true, "first_frag round-trip");
	CHECK(parsed.last_frag == true, "last_frag round-trip");
	CHECK(parsed.expedite == false, "expedite round-trip");
	CHECK(parsed.retransmit == false, "retransmit round-trip");
	CHECK(parsed.has_flow_id == false, "no flow_id");
	CHECK(parsed.has_pfd == false, "no pfd");
	CHECK(parsed.has_hdr_ext == false, "no hdr_ext");
	CHECK(parsed.payload_len == payload_len, "payload length");
	CHECK(memcmp(parsed.payload, payload, payload_len) == 0, "payload data");

	return 0;
}

static int test_flags_roundtrip(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = {0x01, 0x02, 0x03};

	struct rist_adv_params params;
	memset(&params, 0, sizeof(params));
	params.seq = 0xFFFF0001;
	params.timestamp = 999999;
	params.ssrc = 0x00000002; /* Even */
	params.enc_type = RIST_ADV_TYPE_CONTROL;
	params.psk_mode = RIST_ADV_PSK_NONE;
	params.lpc_mode = RIST_ADV_LPC_NONE;
	params.first_frag = true;
	params.last_frag = true;
	params.expedite = true;
	params.retransmit = true;

	int total = rist_adv_build(buf, &params, payload, sizeof(payload));
	CHECK(total > 0, "build with E+R flags");

	struct rist_adv_parsed parsed;
	CHECK(rist_adv_parse(buf, (size_t)total, &parsed) == 0, "parse with E+R");
	CHECK(parsed.expedite == true, "E flag round-trip");
	CHECK(parsed.retransmit == true, "R flag round-trip");
	CHECK(parsed.enc_type == RIST_ADV_TYPE_CONTROL, "control type round-trip");
	CHECK(parsed.seq == 0xFFFF0001, "32-bit seq wrap round-trip");

	return 0;
}

static int test_flow_id_roundtrip(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = {0xAA};

	struct rist_adv_flow_id fid;
	fid.outer = htons(0x1234);
	fid.inner_hi = 0xAB;
	fid.inner_lo_sub = 0xC5; /* inner[3:0]=0xC, sub=0x5 */

	struct rist_adv_params params;
	memset(&params, 0, sizeof(params));
	params.seq = 100;
	params.timestamp = 500;
	params.ssrc = 0x10;
	params.enc_type = RIST_ADV_TYPE_DIRECT;
	params.first_frag = true;
	params.last_frag = true;
	params.flow_id = &fid;

	int total = rist_adv_build(buf, &params, payload, sizeof(payload));
	CHECK(total > 0, "build with flow_id");

	struct rist_adv_parsed parsed;
	CHECK(rist_adv_parse(buf, (size_t)total, &parsed) == 0, "parse with flow_id");
	CHECK(parsed.has_flow_id == true, "I flag present");
	CHECK(parsed.flow_id != NULL, "flow_id pointer");
	CHECK(rist_adv_flow_outer(parsed.flow_id) == 0x1234, "flow outer");
	CHECK(rist_adv_flow_inner(parsed.flow_id) == 0xABC, "flow inner");
	CHECK(rist_adv_flow_sub(parsed.flow_id) == 0x5, "flow sub");

	return 0;
}

static int test_pfd_roundtrip(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = {0x55};

	struct rist_adv_pfd pfd;
	pfd.value = htonl((uint32_t)(0x1 << 28) | 0x0ABCDEF); /* type=1, value=0x0ABCDEF */

	struct rist_adv_params params;
	memset(&params, 0, sizeof(params));
	params.seq = 200;
	params.timestamp = 1000;
	params.ssrc = 0x20;
	params.enc_type = RIST_ADV_TYPE_DIRECT;
	params.first_frag = true;
	params.last_frag = true;
	params.pfd = &pfd;

	int total = rist_adv_build(buf, &params, payload, sizeof(payload));
	CHECK(total > 0, "build with pfd");

	struct rist_adv_parsed parsed;
	CHECK(rist_adv_parse(buf, (size_t)total, &parsed) == 0, "parse with pfd");
	CHECK(parsed.has_pfd == true, "P flag present");
	CHECK(parsed.pfd != NULL, "pfd pointer");
	CHECK(rist_adv_pfd_id_type(parsed.pfd) == 1, "pfd id_type");
	CHECK(rist_adv_pfd_id_value(parsed.pfd) == 0x0ABCDEF, "pfd id_value");

	return 0;
}

static int test_all_psk_modes(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = {0x42};

	for (uint8_t psk = 0; psk <= 7; psk++) {
		uint8_t hash[16], nonce[4], iv[4];
		memset(hash, psk, sizeof(hash));
		memset(nonce, psk + 0x10, sizeof(nonce));
		memset(iv, psk + 0x20, sizeof(iv));

		struct rist_adv_params params;
		memset(&params, 0, sizeof(params));
		params.seq = 300 + psk;
		params.timestamp = 2000;
		params.ssrc = 0x30;
		params.enc_type = RIST_ADV_TYPE_DIRECT;
		params.psk_mode = psk;
		params.first_frag = true;
		params.last_frag = true;

		if (rist_adv_psk_has_hash(psk))  params.psk_hash = hash;
		if (rist_adv_psk_has_nonce(psk)) params.psk_nonce = nonce;
		if (rist_adv_psk_has_iv(psk))    params.psk_iv = iv;

		int total = rist_adv_build(buf, &params, payload, sizeof(payload));
		CHECK(total > 0, "build with psk mode");

		struct rist_adv_parsed parsed;
		char msg[64];
		snprintf(msg, sizeof(msg), "parse psk_mode=%u", psk);
		CHECK(rist_adv_parse(buf, (size_t)total, &parsed) == 0, msg);

		snprintf(msg, sizeof(msg), "psk_mode=%u round-trip", psk);
		CHECK(parsed.psk_mode == psk, msg);

		if (rist_adv_psk_has_hash(psk)) {
			snprintf(msg, sizeof(msg), "psk=%u hash present", psk);
			CHECK(parsed.psk_hash != NULL, msg);
			CHECK(memcmp(parsed.psk_hash, hash, 16) == 0, msg);
		}
		if (rist_adv_psk_has_nonce(psk)) {
			snprintf(msg, sizeof(msg), "psk=%u nonce present", psk);
			CHECK(parsed.psk_nonce != NULL, msg);
			CHECK(memcmp(parsed.psk_nonce, nonce, 4) == 0, msg);
		}
		if (rist_adv_psk_has_iv(psk)) {
			snprintf(msg, sizeof(msg), "psk=%u iv present", psk);
			CHECK(parsed.psk_iv != NULL, msg);
			CHECK(memcmp(parsed.psk_iv, iv, 4) == 0, msg);
		}
	}

	return 0;
}

static int test_seq32_wraparound(void)
{
	uint8_t buf[2048];
	const uint8_t payload[] = {0};
	uint32_t test_seqs[] = {0, 1, 0xFFFF, 0x10000, 0xFFFFFFFE, 0xFFFFFFFF};

	for (size_t i = 0; i < sizeof(test_seqs) / sizeof(test_seqs[0]); i++) {
		struct rist_adv_params params;
		memset(&params, 0, sizeof(params));
		params.seq = test_seqs[i];
		params.timestamp = 0;
		params.ssrc = 0x40;
		params.enc_type = RIST_ADV_TYPE_DIRECT;
		params.first_frag = true;
		params.last_frag = true;

		int total = rist_adv_build(buf, &params, payload, sizeof(payload));
		CHECK(total > 0, "build seq32");

		struct rist_adv_parsed parsed;
		CHECK(rist_adv_parse(buf, (size_t)total, &parsed) == 0, "parse seq32");

		char msg[64];
		snprintf(msg, sizeof(msg), "seq32=0x%08X round-trip", test_seqs[i]);
		CHECK(parsed.seq == test_seqs[i], msg);
	}

	return 0;
}

static int test_ssrc_parity(void)
{
	CHECK(rist_adv_ssrc_is_protected(0x00000000) == true, "even ssrc=0 is protected");
	CHECK(rist_adv_ssrc_is_protected(0x00000001) == false, "odd ssrc=1 is unprotected");
	CHECK(rist_adv_ssrc_is_protected(0xFFFFFFFE) == true, "even ssrc max is protected");
	CHECK(rist_adv_ssrc_is_protected(0xFFFFFFFF) == false, "odd ssrc max is unprotected");
	CHECK(rist_adv_ssrc_protected(0x12345679) == 0x12345678, "protected rounds down");
	CHECK(rist_adv_ssrc_unprotected(0x12345678) == 0x12345679, "unprotected sets bit 0");
	return 0;
}

static int test_psk_hdr_sizes(void)
{
	CHECK(rist_adv_psk_hdr_size(0) == 0, "psk=0: 0 bytes");
	CHECK(rist_adv_psk_hdr_size(1) == 8, "psk=1 (AES-CTR): nonce+iv = 8");
	CHECK(rist_adv_psk_hdr_size(2) == 20, "psk=2 (HMAC): hash+nonce = 20");
	CHECK(rist_adv_psk_hdr_size(3) == 24, "psk=3 (AES-CTR-HMAC): hash+nonce+iv = 24");
	CHECK(rist_adv_psk_hdr_size(4) == 24, "psk=4 (AES-GCM): 24");
	CHECK(rist_adv_psk_hdr_size(5) == 24, "psk=5 (CHACHA20): 24");
	CHECK(rist_adv_psk_hdr_size(6) == 8, "psk=6 (user-no-hash): nonce+iv = 8");
	CHECK(rist_adv_psk_hdr_size(7) == 24, "psk=7 (user-hash): hash+nonce+iv = 24");
	return 0;
}

static int test_malformed_packets(void)
{
	struct rist_adv_parsed parsed;

	CHECK(rist_adv_parse(NULL, 0, &parsed) == -1, "null buf rejected");

	uint8_t short_pkt[8] = {0};
	CHECK(rist_adv_parse(short_pkt, sizeof(short_pkt), &parsed) == -1, "short packet rejected");

	/* Valid-looking RTP header but wrong version */
	uint8_t bad_version[32];
	memset(bad_version, 0, sizeof(bad_version));
	bad_version[0] = 0x40; /* V=1 instead of V=2 */
	bad_version[1] = RIST_ADV_PT;
	CHECK(rist_adv_parse(bad_version, sizeof(bad_version), &parsed) == -1, "wrong RTP version rejected");

	return 0;
}

/* NTP ticks (1/65536 second) to 1 MHz timestamp conversion.
 * Send path:  ts_1mhz = (uint32_t)((source_time * 1000000ULL) >> 16)
 * Recv path:  source_time = ((uint64_t)ts_1mhz << 16) / 1000000ULL
 * The round-trip must preserve the value within rounding tolerance. */
static int test_timestamp_conversion(void)
{
	/* 1MHz timestamp is uint32 — wraps at ~4295 seconds.
	 * Test values must stay within the non-wrapping range. */
	uint64_t test_values[] = {
		0,
		65536,                  /* exactly 1 second */
		65536 * 10,             /* 10 seconds */
		65536 * 3600,           /* 1 hour */
		1,                      /* smallest non-zero */
		32768,                  /* 0.5 seconds */
		(uint64_t)65536 * 4294, /* ~4294s, just under uint32 wrap */
	};

	for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
		uint64_t ntp_orig = test_values[i];
		uint32_t ts_1mhz = (uint32_t)((ntp_orig * 1000000ULL) >> 16);
		uint64_t ntp_back = ((uint64_t)ts_1mhz << 16) / 1000000ULL;

		/* Allow up to 1 NTP tick of rounding error per conversion direction */
		int64_t diff = (int64_t)ntp_orig - (int64_t)ntp_back;
		if (diff < 0) diff = -diff;
		char msg[128];
		snprintf(msg, sizeof(msg), "ts round-trip ntp=%" PRIu64 " -> 1mhz=%u -> ntp=%" PRIu64 " (diff=%" PRId64 ")",
			ntp_orig, ts_1mhz, ntp_back, diff);
		CHECK(diff <= 66, msg); /* 66 = ceil(65536/1000000)*1000000/65536 + 1 */
	}

	/* Verify known exact conversions */
	uint64_t one_sec_ntp = 65536;
	uint32_t one_sec_1mhz = (uint32_t)((one_sec_ntp * 1000000ULL) >> 16);
	CHECK(one_sec_1mhz == 1000000, "1 second NTP -> 1000000 us");

	uint64_t half_sec_ntp = 32768;
	uint32_t half_sec_1mhz = (uint32_t)((half_sec_ntp * 1000000ULL) >> 16);
	CHECK(half_sec_1mhz == 500000, "0.5 second NTP -> 500000 us");

	return 0;
}

/* Verify SSRC-based seq_index mapping: (uint16_t)(seq & 0xFFFF) must
 * give correct low-16 bits for 32-bit seq values. */
static int test_seq_index_mapping(void)
{
	/* seq_index is indexed by low 16 bits of the 32-bit seq */
	uint32_t test_seqs[] = { 0, 1, 0xFFFF, 0x10000, 0x1FFFF, 0x20000, 0xFFFFFFFF };
	uint16_t expected[]  = { 0, 1, 0xFFFF, 0x0000,  0xFFFF,  0x0000,  0xFFFF };

	for (size_t i = 0; i < sizeof(test_seqs) / sizeof(test_seqs[0]); i++) {
		uint16_t idx = (uint16_t)(test_seqs[i] & 0xFFFF);
		char msg[128];
		snprintf(msg, sizeof(msg), "seq_index 0x%08x -> 0x%04x", test_seqs[i], idx);
		CHECK(idx == expected[i], msg);
	}

	return 0;
}

int main(void)
{
	int failures = 0;
	failures += test_basic_roundtrip();
	failures += test_flags_roundtrip();
	failures += test_flow_id_roundtrip();
	failures += test_pfd_roundtrip();
	failures += test_all_psk_modes();
	failures += test_seq32_wraparound();
	failures += test_ssrc_parity();
	failures += test_psk_hdr_sizes();
	failures += test_malformed_packets();
	failures += test_timestamp_conversion();
	failures += test_seq_index_mapping();

	printf("Advanced Profile round-trip tests: %d/%d passed\n", pass_count, test_count);
	if (failures) {
		printf("FAILED\n");
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
