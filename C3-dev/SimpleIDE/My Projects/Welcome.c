/*
    serial echo
*/
#include <stdio.h>
#include <fdserial.h>
#include <propeller.h>

fdserial        *bbb;

int main()                                    // Main function
{
  int      c;
  sleep(1);
  c = 'x';
  printf("sending character <%c>\n",c);
  bbb = fdserial_open(15, 14, 0, 9600);
  writeChar(bbb, CLS);
  while(1){
     fdserial_txChar(bbb,c);
     printf("sending\n");
    sleep(1);
  }    
/*
  while(1)
  {
    c = fdserial_rxChar(bbb);
    if(c != -1)
    {
      printf("You typed: %c\n", c);
    }
  }
  
  */
printf("normal termination\n");                            // Display test message
}  
