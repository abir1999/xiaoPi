#include <stdint.h>

#include "h264_packetizer.h"

static void* rtp_encode_alloc(void* param, int bytes) {
  static uint8_t buffer[2 * 1024 * 1024 + 4] = { 0, 0, 0, 1, };
  if((sizeof(buffer) - 4) <= bytes) {
    printf("rtp package to large\n");
    exit(1);
  }
  return buffer + 4;
}

static void rtp_encode_free(void* param, void *packet) {
}

static int rtp_encode_packet(void* param, const void *packet, int bytes, uint32_t timestamp, int flags) {

  rtp_encode_context_t *rtp_encode_context = (struct rtp_encode_context_t*)param;
  if(!rtp_encode_context->rtc_agent->video_thread_running())
    return 0;

  int ret = peer_connection_send_rtp_packet(rtp_encode_context->rtc_agent->peer_connection(), (uint8_t*)packet, bytes);
  if(ret < bytes)
    rtp_encode_context->rtc_agent->Stop();
  return 0;
}

struct rtp_encode_context_t* create_rtp_encode_context(RtcAgent *rtc_agent) {

  struct rtp_encode_context_t *rtp_encode_context = NULL;
  rtp_encode_context = (rtp_encode_context_t*)malloc(sizeof(struct rtp_encode_context_t));
  rtp_encode_context->handler.alloc = rtp_encode_alloc;
  rtp_encode_context->handler.free = rtp_encode_free;
  rtp_encode_context->handler.packet = rtp_encode_packet;
  rtp_encode_context->encoder = rtp_payload_encode_create(102, "H264", 0, 12345678, &rtp_encode_context->handler, rtp_encode_context);
  rtp_encode_context->rtc_agent = rtc_agent;

  return rtp_encode_context;
}

void rtp_encode_frame(struct rtp_encode_context_t *rtp_encode_context, uint8_t *buf, size_t size, unsigned long timestamp) {
  rtp_payload_encode_input(rtp_encode_context->encoder, buf, size, timestamp);
}

