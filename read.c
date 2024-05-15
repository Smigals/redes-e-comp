/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4
#define STOP 5
#define F 0x5c
#define A 0x03
#define C 0x08
#define BCC1 A^C
#define SET 0x08
#define UA  0x06
#define RR0 0x01
#define RR1 0x11
#define REJ0    0x05
#define REJ1    0x15
#define I0  0x80
#define I1  0xc0
#define DISC    0x0a

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    //unsigned char buf[255];

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    while (state!=STOP) {       /* loop for input */
       unsigned char buf[100];
       res = read(fd,buf,1);   /* returns after 5 chars have been input */
       // buf[res]=0;               /* so we can printf... */
       printf("%02X \n", buf[0]);
        
       switch(state){
            case START:
                if (buf[0] == F){
                    state=FLAG_RCV;
                   
                }
                else {
                    state=START;
                    
                } break;
            case FLAG_RCV:
                if (buf[0] == A){
                    state=A_RCV; 
                        
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                    
                }
                else {
                    state=START;
                    
                } break;
            case A_RCV:
                if (buf[0]==SET){
                    state=C_RCV_SET;
                    BCC1=A^SET;
                    
                }
                else if(buf[0]==DISC){
                    state=C_RCV_DISC;
                    BCC1=A^DISC;
                    
                }
                else if(buf[0]==I0){
                    state=C_RCV_I0;
                    BCC1=A^I0;
                    
                }
                else if(buf[0]==I1){
                    state=C_RCV_I1;
                    BCC1=A^I1;
                } 
                else if(buf[0]==UA){
                    state=C_RCV_UA;
                    BCC1=A^UA;
                }  
                else if(buf[0]==F){
                    state=FLAG_RCV;
                   
                }   
                else {
                    state=START;
                } break;

            case (C_RCV_SET || C_RCV_DISC || C_RCV_UA):
                if(buf[0]==BCC1){
                    state=BCC1_OK;
                    
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case BCC1_OK:
                if (buf[0]==F){
                    state=STOP;
                   
                }
                else {
                    state=START;
                   
                } break;
            case C_RCV_I0:
                if(buf[0]==
        }
            
  //      if (buf[0]=='z') break;
    }
    unsigned char buf2[5]={0x5c, 0x01, 0x06, 0x01^0x06, 0x5c};
    res=write(fd, buf2, 5);
    printf("\n\n%d\n", res);
    for(int i=0; i<5; i++) {
        printf("%02x\n", buf2[i]);
    

    }

    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */
    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
