/*
 * ASIF file encoder
 *
 * Nate Watanabe & Jonathan Vidal-Contreras
 * April 20, 2020
 */

#include "avcodec.h"
#include "internal.h"
#include "bytestream.h"
#include "../libavformat/avio.h"

/*
 * These nodes will create a linked list that will hold all the audio
 * info received from the AVFrames
 */
typedef struct asif_node{
  uint8_t *data[16]; // each entry in the array is a pointer to channel data
  struct asif_node *next;
  int num_samples; // keep track of number of samples for each frame
} asif_node;


/*
 * The private data utilized by the encoder 
 */
typedef struct asif_encode_data{
  int num_channels;
  int total_samples; 
  int drained;
  int received_all_frames, draining;
  asif_node *head; // keep track of the head of the linked list
} asif_encode_data;

/*
 * Method Declarations
 */
static asif_node* add_asif_node(asif_encode_data *pd);
static void gen_deltas (asif_encode_data *pd, uint8_t* deltas, int channel_number);

/*
 * Adds an asif_node to the linked list containing audio data.
 * Returns a pointer to the new node that was added.
 */ 
static asif_node* add_asif_node(asif_encode_data *pd){

  asif_node *curr, *prev;
  
  if (pd->head == NULL){ // add the first node to the linked list
    pd->head = (asif_node*) av_mallocz(sizeof(asif_node));
    return pd->head;
  }

  curr = pd->head;
  while(curr){
    prev = curr;
    curr = curr->next;
  }
  curr = (asif_node*) av_mallocz(sizeof(asif_node));

  if (prev) // if there is a node before the new node
    prev->next = curr;

  curr->next = NULL;

  return curr;
}

/*
 *Writes in an initial sample and corresponding deltas based on the initial sample.
 *@param deltas -- size of deltas will be size of one channel
 */
static void gen_deltas(asif_encode_data *pd, uint8_t* deltas, int channel_number){
  int i, pos;
  uint8_t curr_sample;
  int curr_delta;
  asif_node *curr;
 
  deltas[0] = pd->head->data[channel_number][0]; // the initial sample
  curr_sample = deltas[0];
  curr = pd->head;
  pos = 0; // initial offset is 0 (start writing at the beginning of the array)
  i = 1;

  while (curr) { // go through the data obtained from the frames
    for (; i < curr->num_samples; i++){
     
      // generate deltas based on consecutive samples
      curr_delta = (int) curr->data[channel_number][i] - (int) curr_sample; 

      // clamp the value
      if (curr_delta > 127){
	curr_delta = 127;
      }
      else if (curr_delta < -128){
	curr_delta = -128;
      }
      
      // place the delta in its place in the array
      deltas[i + pos] = (uint8_t) curr_delta;
      curr_sample = curr_sample + curr_delta;
    }
    pos += curr->num_samples; // offset to write in info from next node/frame
    curr = curr->next; // move to next node/frame
    i = 0;
  }
}

/*
 * Sets the frame size, as well as the fields of the asif_encode_data struct.
 * Since frame_size is the number of samples per channel per frame, if there are
 * 2 channels with frame_size 1,000,000, there will be 2,000,000 samples per frame.
 */
static int asif_encode_init(AVCodecContext *avctx){ 
  
  asif_encode_data *s = avctx->priv_data;
  s->total_samples = 0;
  s->received_all_frames = 0; // change to one once all frame data is collected
  s->drained = 0; // change to 1 to indicate buffer is drained
  avctx->frame_size = 1000000; // number of samples per channel per frame
 
  return 0;
}

/*
 * FFMPEG calls this to send the decoded audio frames to the encoder. 
 * Since the frames are in a planar format, must use frame->extended_data.
 */
static int asif_send_frame (AVCodecContext *avctx, const AVFrame *frame){
 
  asif_encode_data *s;
  asif_node *new_node = NULL;

  s = avctx->priv_data;
  s->draining = 0;

  if (!frame) { // if frame is null
    s->draining = 1;
    if (s->draining){
      s->drained = 1;
      return 0;
    }
    return AVERROR_EOF;
  }
  else {
    if (!s->draining)	{ // FFMPEG is still sending non-null frames

      s->num_channels = frame->channels;
      // increment number of total samples found for each frame
      s->total_samples += frame->nb_samples * frame->channels;
       
      new_node = add_asif_node(s); // add the node to the existing list
      new_node->num_samples = frame->nb_samples;

      for(int c = 0; c < s->num_channels; c++){
	new_node->data[c] = av_memdup(frame->extended_data[c], new_node->num_samples);
      }

      if (s->head == NULL)
	s->head = new_node;

      return 0;
    }
  }
 
  return AVERROR(EAGAIN);
}

/*
 * After receiving all the data from the frames, this method will write
 * the received information to an AVPacket for later use in the muxer.
 */
static int asif_receive_packet(AVCodecContext *avctx, AVPacket *avpkt){

  asif_encode_data *s;
  int packet_size, ret, total_samples_per_channel;
  uint8_t *beg_buf, *deltas;

  s = avctx->priv_data;

  // + 4 bytes to account for writing part of the header
  packet_size = s->total_samples + 4; // should be the number of total samples in the file  
  
  if (!s->received_all_frames && s->drained) { // start collecting all the frames data
    
    if ((ret = ff_alloc_packet2(avctx, avpkt, packet_size, packet_size)) < 0)
      return ret; // error while allocating packet

    total_samples_per_channel = s->total_samples / s->num_channels;
   
    beg_buf = avpkt->data;

    bytestream_put_le32(&avpkt->data, total_samples_per_channel); // write in number of samples per channel
   
    for (int c = 0; c < s->num_channels; c++){ // Walk through the channel's data one by one
      
      // allocate space for the array of deltas
      deltas = (uint8_t*) av_mallocz(total_samples_per_channel); // size of an entire channel
      
      // walk through the samples, calculate deltas, and put them into the array to be copied
      gen_deltas(s, deltas, c); 
      // write in all the delta information to the packet
      bytestream_put_buffer(&avpkt->data, deltas, total_samples_per_channel);
     
      av_free(deltas);
    }

    avpkt->data = beg_buf; // have the packet's data point to the beginning of the buffer
      
    s->received_all_frames = 1; // set the flag to true
    return 0;
  }
  if (s->drained)
    return AVERROR_EOF;
  return AVERROR(EAGAIN);
}

/*
 * Cleans up any allocated memory.
 */
static int asif_encode_close(AVCodecContext *avctx){
 
  asif_encode_data *s;
  asif_node *curr, *temp;

  s = avctx->priv_data;

  curr = s->head;
  while (curr) // free each node in the linked list
    {
      temp = curr;
      curr = curr->next;
      av_free(temp);
    }

  return 0;
}

AVCodec ff_asif_encoder = {
  .id             = AV_CODEC_ID_ASIF,
  .priv_data_size = sizeof(asif_encode_data),
  .type           = AVMEDIA_TYPE_AUDIO,
  .name           = "asif",
  .long_name      = NULL_IF_CONFIG_SMALL("ASIF audio file (CS 3505 Spring 20202)"),
  .init           = asif_encode_init,
  .send_frame     = asif_send_frame,
  .receive_packet = asif_receive_packet,
  .close          = asif_encode_close,
  .sample_fmts    = (const enum AVSampleFormat[]) {AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_NONE},
  .capabilities   = AV_CODEC_CAP_DELAY, 
};
