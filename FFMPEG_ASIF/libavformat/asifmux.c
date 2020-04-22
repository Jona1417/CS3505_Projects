/*
 * ASIF muxer
 *
 * Nate Watanabe & Jonathan Vidal-Contreras
 * April 20, 2020
 */

#include "avformat.h"
#include "avio.h"
#include "../libavcodec/avcodec.h"
#include "../libavutil/avutil.h"

/*
 * Writes the header information to the AVIOContext of the input
 */
static int asif_write_header(AVFormatContext *s)
{
  AVIOContext *pb = s->pb;
  AVCodecParameters *params = s->streams[0]->codecpar;

  // Write in the "asif" tag
  const char asif[4] = "asif";
  avio_write(pb, asif, 4); 
  
  // Write in the sample rate {32-bit little endian int}
  avio_wl32(pb, params->sample_rate);
  
  // Write in # of channels {16-bit little endian int}
  avio_wl16(pb, params->channels);

  /* # of samples per channel is already written in the encoder */

  return 0;
}

/* Writes from a data packet to an AVFormatContext */
static int asif_write_packet(AVFormatContext *s, AVPacket *pkt)
{
  AVIOContext *pb = s->pb;

  avio_write(pb, pkt->data, pkt->size);

  return 0;
}

AVOutputFormat ff_asif_muxer = {
    .name              = "asif",
    .long_name         = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
    .mime_type         = "audio",
    .extensions        = "asif",
    .audio_codec       = AV_CODEC_ID_ASIF,
    .video_codec       = AV_CODEC_ID_NONE,
    .write_header      = asif_write_header,
    .write_packet      = asif_write_packet,
};
