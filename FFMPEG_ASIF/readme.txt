(2020/4/22)
This README file is to describe to viewers of my FFMPEG project the various files contained within.

ASIF - "Audio Slope Information Format" is an audio format created by Prof. Jensen in the
Spring 2020 Semester of CS3505 at the University of Utah. The basic idea is that the format has
a 14-byte header describing the tag 'asif', sample rate, number of channels, and number of samples
per channel.

The format of the audio information is planar. Given n samples per channel, an unsigned 8-bit integer
indicates the initial sample value, followed by (n -1) "deltas" (accounting for overflow and "catching
up" with the following deltas).

bach.mp3 is just an mp3 file we used for testing our codec.
my_output.asif is the output .asif file we generated from bach.mp3
my_output.wav is the output .wav file we generated from using our demuxer/decoder on my_output.asif.
