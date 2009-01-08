/* Ugly password generator 
 * FIXME: slightly non-uniform distribution of result characters
 */

#include <stdio.h>
#include <fcntl.h>

char *table[] = {"0123456789",
                 "0123456789abcdef",
                 "0123456789ABCDEF",
                 "0123456789ABCDEFGHJKLMNPRSTUVXYZ",
                 "0123456789abcdefghjklmnprstuvxyz",
                 "abcdefghjkmnprstuvxABCDEFGHJKMNPRSTUVX0123456789",
                 "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,."};

int main(int argc, char**argv){
  int num;
  int len;
  unsigned char symnum;

  if (argc == 3){
    if (strcmp(argv[1], "numeric") == 0){
      num = 0;
    } else if (strcmp(argv[1], "hex") == 0){
      num = 1;
    } else if (strcmp(argv[1], "HEX") == 0){
      num = 2;
    } else if (strcmp(argv[1], "dec32") == 0){
      num = 3;
    } else if (strcmp(argv[1], "DEC32") == 0){
      num = 4;
    } else if (strcmp(argv[1], "cz") == 0){
      num = 5;
    } else if (strcmp(argv[1], "b64") == 0){
      num = 6;
    } else {
      num = 6;
    }
    len = atoi(argv[2]);
  } else if (argc == 2){
    len = atoi(argv[1]);
    num = 6;
  } else {  
    fputs("usage: passgen [numeric|hex|HEX|dec32|DEC32|cz|b64] <length>\n",stderr);
    return 1;
  }
    
  


  symnum = strlen(table[num]);
  int fd = open("/dev/random", O_RDONLY);

  while (len){
    unsigned char buf;
    read(fd, &buf, 1);
    putchar(table[num][buf % symnum]);
    --len;
  }
  
  puts("\n");
  
}
