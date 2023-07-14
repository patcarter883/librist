
#include "gre.h"
#include "common/attributes.h"
#include "config.h"
#include "crypto/psk.h"
#include "log-private.h"
#include "logging.h"
#include "rist-private.h"
#include "endian-shim.h"
#include "udp-private.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


ssize_t rist_send_data_main_profile(struct rist_peer *p, uint8_t payload_type, uint16_t proto, uint8_t *payload, size_t payload_len, uint16_t src_port, uint16_t dst_port) {
	bool encrypt = (p->key_tx.key_size > 0);

	/* Our encryption and compression operations directly modify the payload buffer we receive as a pointer
	   so we create a local pointer that points to the payload pointer, if we would either encrypt or compress we instead
	   malloc and mempcy, to ensure our source stays clean. We only do this with RAW data as these buffers are the only
	   assumed to be reused by retransmits */
    bool modifying_payload = encrypt && (payload_type == RIST_PAYLOAD_TYPE_DATA_RAW || payload_type == RIST_PAYLOAD_TYPE_DATA_RAW_RTP_EXT);//Need to make a copy of our data if we'd resend it in the future, otherwise we can safely overwrite the buffer

	uint8_t *payload_wr = payload;
	uint8_t hdr_buf[MAX_GRE_SIZE];
	struct rist_gre_hdr* hdr = (struct rist_gre_hdr *)hdr_buf;
	memset(hdr, 0, sizeof(*hdr));
	size_t hdr_len = sizeof(*hdr);

	uint32_t seq = p->seq++;
    assert(payload != NULL);

	const size_t nonce_offset = sizeof(*hdr);
	hdr->flags2 = (p->rist_gre_version &0x7) << 3;

	if (encrypt) {
		hdr_len += 4;
		if (p->rist_gre_version && p->key_tx.key_size == 256)
		{
			hdr->flags2 |= (1 & 1UL) << 6;
		}
	}
	//VSF TR-06-02:_2022-08-11 wants us to send a sequence if we're capable of sending non-RIST traffic
	//For now default to always considering us capable to do that.
	if (true || encrypt) {
		SET_BIT(hdr->flags1, 4); // set seq bit
		//Write sequence number
		hdr_buf[hdr_len] = seq >> 24;
		hdr_buf[hdr_len+1] = seq >> 16;
		hdr_buf[hdr_len+2] = seq >> 8;
		hdr_buf[hdr_len+3] = seq;
		hdr_len += 4;
	}

	size_t hdr_payload_offset = hdr_len;

	//If we can use the new VSF ethertype (rist GRE version >= 2), we should, as the other way is considered deprecated
	if (p->rist_gre_version >= 2 && (proto == RIST_GRE_PROTOCOL_TYPE_REDUCED || proto == RIST_GRE_PROTOCOL_TYPE_KEEPALIVE)) {
		struct rist_vsf_proto *vsf = (struct rist_vsf_proto *)&hdr_buf[hdr_len];
		hdr_len += sizeof(*vsf);
		vsf->type = RIST_VSF_PROTOCOL_TYPE_RIST;//Network byte order! these are 0 values, so safe to do without htobe16
		if (proto == RIST_GRE_PROTOCOL_TYPE_REDUCED)
			vsf->subtype = RIST_VSF_PROTOCOL_SUBTYPE_REDUCED;
		else
			vsf->subtype = htobe16(RIST_VSF_PROTOCOL_SUBTYPE_KEEPALIVE);
		proto = RIST_GRE_PROTOCOL_TYPE_VSF;
		hdr->prot_type = htobe16(RIST_GRE_PROTOCOL_TYPE_VSF);
	} else {
		hdr->prot_type = htobe16(proto);
	}

	if (proto == RIST_GRE_PROTOCOL_TYPE_REDUCED) {
		struct rist_reduced *red = (struct rist_reduced *)(&hdr_buf[hdr_len]);
		hdr_len += sizeof(*red);
		red->dst_port = htobe16(dst_port);
		red->src_port = htobe16(src_port);
	}

	if (encrypt) {
		if (modifying_payload || (hdr_payload_offset != hdr_len && !HAVE_MBEDTLS)) {
			payload_wr = malloc(payload_len + 8);//Let's get rid of this malloc in the hotpath here
			modifying_payload = true;
			assert(payload_wr);
		}
		//Data that should be encrypted is part of the header
		if (hdr_payload_offset != hdr_len) {
#if HAVE_MBEDTLS
			//MbedTLS allows us to continue encryption, so we can encrypt hdr & payload in 2 ops
			_librist_crypto_psk_encrypt(&p->key_tx, htobe32(seq), p->rist_gre_version, &hdr_buf[hdr_payload_offset], &hdr_buf[hdr_payload_offset], hdr_len - hdr_payload_offset);
			_librist_crypto_psk_encrypt_continue(&p->key_tx, payload, payload_wr, payload_len);
#else
			memcpy(payload_wr, &hdr_buf[hdr_payload_offset], hdr_len - hdr_payload_offset);
			memcpy(&payload_wr[hdr_len - hdr_payload_offset], payload, payload_len);
			payload_len += (hdr_len - hdr_payload_offset);
			hdr_len -= (hdr_len - hdr_payload_offset);
			_librist_crypto_psk_encrypt(&p->key_tx, htobe32(seq), p->rist_gre_version, payload_wr, payload_wr, payload_len);
#endif
		} else {
			//Single encryption pass suffices
			_librist_crypto_psk_encrypt(&p->key_tx, htobe32(seq), p->rist_gre_version, payload, payload_wr, payload_len);
		}

		SET_BIT(hdr->flags1, 5); // set key bit
		//Write key, our nonce is stored in network byte order (even though it's uint32_t), so just memcpy it
		memcpy(&hdr_buf[nonce_offset], &p->key_tx.gre_nonce, sizeof(p->key_tx.gre_nonce));
	}

	ssize_t ret;
	int errorcode = 0;

	//TODO: abstract this away
#ifndef _WIN32
	//TODO: this is POSIX only: add windows equivalent
	struct msghdr msghdr;
	struct iovec iov[2];
	iov[0].iov_base = hdr_buf;
	iov[0].iov_len = hdr_len;
	iov[1].iov_base = payload_wr;
	iov[1].iov_len = payload_len;
	msghdr.msg_iov = iov;
	msghdr.msg_iovlen = 2;
	msghdr.msg_name = &p->u.address;
	msghdr.msg_namelen = p->address_len;
	msghdr.msg_control = NULL;
	msghdr.msg_controllen = 0;
	msghdr.msg_flags = 0;
	ret = sendmsg(p->sd, &msghdr, MSG_DONTWAIT);
	if (RIST_UNLIKELY(ret < 0)) {
		errorcode = errno;
	}
#else
	WSAMSG msghdr = { 0 };
	WSABUF iov[2];
	DWORD dwBytes = 0;
	iov[0].len = hdr_len;
	iov[0].buf = (char *)hdr_buf;
	iov[1].len = payload_len;
	iov[1].buf = (char *)payload_wr;
	msghdr.name = (struct sockaddr *)&p->u.address;
	msghdr.namelen = p->address_len;
	msghdr.lpBuffers = iov;
	msghdr.dwBufferCount = 2;

	ret = payload_len + hdr_len;
	if (RIST_UNLIKELY(WSASendMsg(p->sd, &msghdr, MSG_DONTWAIT, &dwBytes, NULL, NULL) != 0)) {
		ret = -1;
		errorcode = WSAGetLastError();
	}
#endif

	if (modifying_payload) {
		free(payload_wr);
	}

	if (RIST_UNLIKELY(errorcode)) {
        struct rist_common_ctx *ctx = get_cctx(p);
        rist_log_priv(ctx, RIST_LOG_ERROR, "Send failed: errno=%d, reason=%s, ret=%d, socket=%d\n", errorcode, strerror(errorcode), ret, p->sd);
	}

	return ret;
}