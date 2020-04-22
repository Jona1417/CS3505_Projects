/*
 * ASIF decoder
 *
 * Nate Watanabe & Jonathan Vidal-Contreras
 * April 20, 2020
 */

#include <inttypes.h>
#include "avcodec.h"
#include <stdio.h>
#include "bytestream.h"
#include "decode.h"
#include "internal.h"

/*
 * Takes the sample and delta information in ASIF files and 
 * recalculates the samples accordingly
 */
static void decode_deltas(const uint8_t *deltas, AVFrame *frame);

static void decode_deltas(const uint8_t *deltas, AVFrame *frame){
  int i, pos;
  uint8_t sample; // initial sample which we use to calculate next samples
  uint8_t *output;
  
  pos = 0;

  for(int c = 0; c < frame->channels; c++){ // go through each channel
    output = frame->extended_data[c]; 
    sample = deltas[pos];
    output[0] = sample;

    for (i = 1; i < frame->nb_samples; i++){
      output[i] = sample + deltas[pos + i]; 
      sample += deltas[pos + i];
     }
    pos += frame->nb_samples; // increment the offset
  }
}

/*
 * Takes the data from the AVPacket and converts it from delta form to
 * sample form, and puts it in the frame.
 */
static int asif_decode_frame(AVCodecContext *avctx, void *outdata,
                            int *got_frame_ptr, AVPacket *pkt)
{
  int ret;
  const uint8_t *buf = pkt->data;
  AVFrame *frame = outdata;

  // read the tag asif
  if(buf[0] != 'a' || buf[1] != 's' || buf[2] != 'i' || buf[3] != 'f'){
    return AVERROR(EINVAL);
  }

  buf += 4; // move past the tag

  // read header and set the parameters for the frames
  frame->sample_rate = bytestream_get_le32(&buf);
  frame->channels    = bytestream_get_le16(&buf);
  frame->nb_samples  = bytestream_get_le32(&buf);

  // set avctx parameters
  avctx->codec_id   = AV_CODEC_ID_ASIF;
  avctx->sample_fmt = AV_SAMPLE_FMT_U8P;
  avctx->codec_type = AVMEDIA_TYPE_AUDIO;
  avctx->sample_rate = frame->sample_rate;
  avctx->channels = frame->channels;

  ret = ff_get_buffer(avctx, frame, 0);

  if (ret < 0)
    return ret;

  // decode deltas, write them into the frame
  decode_deltas(buf, frame);
  
  *got_frame_ptr = 1;

  return pkt->size;
}

AVCodec ff_asif_decoder = {
  .id             = AV_CODEC_ID_ASIF,
  .type           = AVMEDIA_TYPE_AUDIO,
  .name           = "asif",
  .long_name      = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
  .decode         = asif_decode_frame,
  .sample_fmts    = (const enum AVSampleFormat[]) {AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_NONE},
};
