/* play DTMF tones to OSS soundcard */

#include <unistd.h>
#include <math.h>
#include <sys/soundcard.h>
#include <stdio.h>
#include <fcntl.h>

int rows[4] = {697, 770, 852, 941};
int cols[4] = {1209, 1336, 1477, 1633};


void play_num(int fd, char num){
  signed char buf[800];
  int row, col, k;

  if (num >= '1' && num <= '9'){
    row = (num - '1') / 3;
    col = (num - '1') % 3;
  } else if (num >= 'a' && num <= 'd'){
    row = num - 'a';
    col = 3;
  } else if (num == '0') {
    row = 3;
    col = 1;
  } else if (num == '*') {
    row = 3;
    col = 0;
  } else if (num == '#') {
    row = 3;
    col = 2;
  } else if (num == '\n') {
    ioctl(fd, SNDCTL_DSP_SYNC);
    return;
  } else if (num == 'p') {
    memset(buf, 0, 800);
    write(fd, &buf, 800);
    return;
  } else {
    return;
  }

  for (k = 0; k < 800; k++){
    buf[k] = (signed char)63*(sin(2*M_PI*rows[row]*k*0.000125)+
                              sin(2*M_PI*cols[col]*k*0.000125));
  }
  write(fd, &buf, 800);

  memset(buf, 0, 320);
  write(fd, &buf, 320);

}

int main(int argc, char**argv){
  int param;
  int fd;

  fd = open("/dev/dsp", O_RDWR);

  if (fd < 0){
    perror("/dev/dsp");
  }

  param = AFMT_S8;
  ioctl(fd, SNDCTL_DSP_SETFMT, &param);
  param = 1;
  ioctl(fd, SNDCTL_DSP_CHANNELS, &param);
  param = 8000;
  ioctl(fd, SNDCTL_DSP_SPEED, &param);
  
  while (!feof(stdin)){
    play_num(fd, getchar());
  }
}
