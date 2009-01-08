/*
 * byc - Measure amount of data on stdin (and either sink it or pass to stdout).
 * Copyright (c) 2006-2008 Ales Hakl
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

#define BUF_SIZE 1048576

int human_readable = 0;
int pass_thru = 0;

void format(long long val, double*out, char**prefix){
  if (val < 1024){
    *out = (double)val;
    *prefix = "";
    return;
  }
  if (val < 1048576){
    *out = (double)val / 1024;
    *prefix = "Ki";
    return;
  }
  if (val < 1073741824){
    *out = (double)val / 1048576;
    *prefix = "Mi";
    return;
  }

  if (val < ((long long)1073741824)*1024){
    *out = (double)val / 1073741824;
    *prefix = "Gi";
    return;
  }

  *out = (double)val / ((long long)1073741824)*1024;
  *prefix = "Ti";
  return;

}

int my_write(int fd, char* buf, size_t len){
  ssize_t ret;

  while (len){
    ret = write(fd, buf, len);
    if (ret == -1){
      return 0;
    }
    buf += ret;
    len -= ret;
  }
  return 1;
}

int main (int argc, char**argv){

  ssize_t r;
  char buf[BUF_SIZE];
  time_t start;
  time_t last;
  time_t now;
  long long speed = 0;
  long long avg_speed = 0;
  long long sum = 0;
  long long l_sum = 0;
  int c;

  while ((c=getopt(argc, argv, "ph")) != -1){
    switch (c){  
    case 'p':
      pass_thru = 1;
      break;
    case 'h':
      human_readable = 1;
      break;
    default:
      printf("Usage: %s [-ph]\n", argv[0]);
      return 2;
    }
  }
  
  if (optind != argc){
    printf("Usage: %s [-ph]\n", argv[0]);
    return 2;
  }


  time(&start);
  last = start;
  while ((r = read(0, buf, BUF_SIZE))>0){
    if (pass_thru){
      if (!my_write(1, buf, r)){
        perror("write");
        return 1;
      }
    }
    time(&now);
    
    sum += r;

    if (now != last){
      speed = (sum - l_sum) / (now - last);
      avg_speed = (sum) / (now - start);

      l_sum = sum;
      last = now;
    }

    if (human_readable){
      double v_sum;
      double v_speed;
      double v_avg_speed;

      char* p_sum;
      char* p_speed;
      char* p_avg_speed;

      format(sum, &v_sum, &p_sum);
      format(speed, &v_speed, &p_speed);
      format(avg_speed, &v_avg_speed, &p_avg_speed);

      fprintf(stderr, "\r\033[2K"
              "Transfered: %.3f %sB, Speed: %.3f %sB/s "
              "(Average: %.3f %sB/s)",
              v_sum, p_sum, v_speed, p_speed, v_avg_speed, p_avg_speed);
    }else{
      fprintf(stderr, "\r\033[2K"
              "Transfered: %lld B, Speed: %lld B/s "
              "(Average: %lld B/s)",
              sum, speed, avg_speed);

    }

  }

  if (r < 0){
    perror("read");
    return 1;
  }

  time(&now);

  fprintf(stderr, "\nDone: %lld bytes in %d seconds\n", sum, now-start);


  return 0;
}
