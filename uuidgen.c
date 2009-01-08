/* generate v4 UUID */

#include <stdio.h>
#include <fcntl.h>


int main(int argc, char** argv){
  unsigned char uuid[16];
  int fd;
  fd = open("/dev/random", O_RDONLY);
  if (fd == -1){
    perror("open");
    return 1;
  }
  read(fd, uuid, 16);
  uuid[6] &= 0x0f;
  uuid[6] |= 0x40;
  uuid[8] &= 0x3f;
  uuid[8] |= 0x80;
  printf("%02x%02x%02x%02x-%02x%02x-"
         "%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
         uuid[0], uuid[1], uuid[2], uuid[3], 
         uuid[4], uuid[5], uuid[6], uuid[7],
         uuid[8], uuid[9], uuid[10], uuid[11],
         uuid[12], uuid[13], uuid[14], uuid[15]);
  return 0;
}
