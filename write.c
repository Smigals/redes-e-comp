    /*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    //char buf[255];
    int i, sum = 0, speed = 0;

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

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



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



    /*for (i = 0; i < 5; i++) {
        buf[i] = 'a';
    }*/
   
    unsigned char buf[5] ={0x5c, 0x03, 0x08, 0x03^0x08, 0x5c};
    res = write(fd,buf,5);
    printf("%d ", res);
    for(int i=0; i<5;i++){
        printf("%02X ", buf[i]);
    }
     
    /*testing*/    

    printf("\n%d bytes written\n", res);
  //while (state!=STOP) {       /* loop for input */
      // unsigned char buf[1];
       //res = read(fd,buf,1);   /* returns after 5 chars have been input */
       // buf[res]=0;               /* so we can printf... */
       /*printf("%02X \n", buf[0]);
        
       switch(state){
            case START:
                if (buf[0] == F){
                    state=FLAG_RCV;
                    printf("flag_rcv\n");
                }
                else {
                    state=START;
                    printf("start\n");
                } break;
            case FLAG_RCV:
                if (buf[0] == A){
                    state=A_RCV; 
                    printf("a_rcv\n") ;      
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                    printf("flag go back\n");
                }
                else {
                    state=START;
                    printf("start\n");
                } break;
            case A_RCV:
                if (buf[0]==C){
                    state=C_RCV;
                    printf("c_rcv\n");
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                    printf("a go back\n");
                }
                else {
                    state=START;
                    printf("start\n");
                } break;
            case C_RCV:
                if (buf[0]==BCC){
                    state=BCC_OK;
                    printf("BCC_OK\n");
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                    printf("c go back\n");
                }
                else {
                    state=START;
                    printf("start\n");
                } break;
            case BCC_OK:
                if(buf[0]==F){
                    state=STOP;
                    printf("stop\n");
                }
                else {
                    state=START;
                    printf("start\n");
                } break;
        }
            */
  //      if (buf[0]=='z') break;
    //}

    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */
    
    sleep(1);
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

	
    close(fd);
    return 0;
}
