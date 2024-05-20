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
#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4
#define STOP 5
#define F 0x5c
#define A 0x01
#define SET 0x08
#define UA  0x06
#define RR0 0x01
#define RR1 0x11
#define REJ0    0x05
#define REJ1    0x15
#define I0  0x80
#define I1  0xc0
#define DISC    0x0a
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
   
    unsigned char buf[5] ={0x5c, 0x03, 0x08, 0x03^0x08,0x5c};
    res = write(fd,buf,5);
    printf("%d ", res);
    for(int i=0; i<5;i++){
        printf("%02X ", buf[i]);
    }
    
    /*testing*/    

    printf("\n%d bytes written\n", res);
    
    while (state!=STOP) {       /* loop for input */
      unsigned char buf2[1];
       res = read(fd,buf2,1);   /* returns after 5 chars have been input */
       buf2[res]=0;               /* so we can printf... */
       
        
       switch(state){
            case START:
                if (buf2[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x\n", buf2[0]);
                }
                else {
                    state=START;
                    //printf("F=%02x\n", buf2[0]);
                } break;
            case FLAG_RCV:
                if (buf2[0] == A){
                    state=A_RCV; 
                    printf("A=%02x\n", buf2[0]);      
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                    printf("A=%02x\n", buf2[0]);
                }
                else {
                    state=START;
                    printf("A=%02x\n", buf2[0]);
                } break;
            case A_RCV:
                if (buf2[0]==C){
                    state=C_RCV;
                    printf("C=%02x\n", buf2[0]);
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                    printf("C=%02x\n", buf2[0]);
                }
                else {
                    state=START;
                    printf("C=%02x\n", buf2[0]);
                } break;
            case C_RCV:
                if (buf2[0]==BCC){
                    state=BCC1_OK;
                    printf("BCC=%02x\n", buf2[0]);
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                    printf("BCC=%02x\n", buf2[0]);
                }
                else {
                    state=START;
                    printf("BCC=%02x\n", buf2[0]);
                } break;
            case BCC1_OK:
                if(buf2[0]==F){
                    state=STOP;
                    printf("FLAG=%02x\n", buf2[0]);
                }
                else {
                    state=START;
                    printf("FLAG=%02x\n", buf2[0]);
                } break;
        }
        
    }
   
    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */
    unsigned char buf[10] ={0x5c, 0x03, 0x80, 0x03^0x80, 0x9b, 0x61, 0xa1, 0xff, ((0x9b^0x61)^0xa1)^0xff,0x5c};
    res = write(fd,buf,10);
    printf("%d ", res);
    for(int i=0; i<10;i++){
        printf("%02X ", buf[i]);
    }
    state=START;


    sleep(1);
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

	
    close(fd);
    return 0;
}
