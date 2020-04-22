/*
 * ASIF demuxer
 *
 * Nate Watanabe & Jonathan Vidal-Contreras
 * April 20, 2020
 */
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "avformat.h"

static int asif_read_header(AVFormatContext *s){
  
  AVStream *st = avformat_new_stream(s, NULL);
  if (!st)
    return AVERROR(ENOMEM);
  st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO; // set codec type
  st->codecpar->codec_id = s->iformat->raw_codec_id; // set codec ID
  st->start_time = 0;
    return 0;
}

/*
 * Reads data from the AVIOContext to an AVPacket
 */
static int asif_read_packet(AVFormatContext *s, AVPacket *pkt){
   
  int ret, size;
  size = avio_size(s->pb); // prepare size of the AVPacket

  if ((ret = av_new_packet(pkt, size)) < 0)
    return ret;
  
  pkt->pos = avio_tell(s->pb);
  pkt->stream_index = 0;

  // read the data into the packet
  ret = avio_read(s->pb, pkt->data, size);

  if (ret < 0) {
    av_packet_unref(pkt);
    return ret;
  }

  av_shrink_packet(pkt, ret);
  return ret;
}

AVInputFormat ff_asif_demuxer = {
  .name           = "asif",
  .priv_data_size = 0,
  .long_name      = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
  .extensions     = "asif",
  .read_header    = asif_read_header,
  .read_packet    = asif_read_packet,
  .raw_codec_id   = AV_CODEC_ID_ASIF,
};
