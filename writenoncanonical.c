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
#define BCC1_OK 4
#define STOPP 5
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
#define C_RCV_SET 10
#define C_RCV_DISC 11
#define C_RCV_RR0 12
#define C_RCV_RR1 13
#define C_RCV_REJ0 14
#define C_RCV_REJ1 15
#define ESC_OCT1 0x5d
#define ESC_OCT2 0x7c
#define ESC_OCT3 0x7d
volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, state;
    unsigned char BCC1;
    struct termios oldtio,newtio;
    //char buf[255];
    int i, sum = 0, speed = 0;

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS11\n");
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
    //ENVIO DO SET
    state = START;
    while (state!=STOPP) {       /* loop for input */
      unsigned char buf2[1];
       res = read(fd,buf2,1);   /* returns after 5 chars have been input */
       buf2[res]=0;               /* so we can printf... */
       
        
       switch(state){
            case START:
                if (buf2[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x ", buf2[0]);
                    fflush(stdout);
                }
                else {
                    state=START;
                    //printf("F=%02x\n", buf2[0]);
                } break;
            case FLAG_RCV:
                if (buf2[0] == A){
                    state=A_RCV; 
                    printf("A=%02x ", buf2[0]);
                    fflush(stdout);      
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                    
                }
                else {
                    state=START;
                    
                } break;
            case A_RCV:
                if (buf2[0]==UA){
                    state=C_RCV;
                    printf("C=%02x ", buf2[0]);
                    fflush(stdout);
                    BCC1=A^UA;
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                   
                }
                else {
                    state=START;
                   
                } break;
            case C_RCV:
                if (buf2[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf2[0]);
                    fflush(stdout);
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                    
                }
                else {
                    state=START;
                    
                } break;
            case BCC1_OK:
                if(buf2[0]==F){
                    state=STOPP;
                    printf("FLAG=%02x\n", buf2[0]);
                    fflush(stdout);
                }
                else {
                    state=START;
                    
                } break;
        }
        
    }
   
    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */
    //dados
    unsigned char buf3[10] ={0x5c, 0x03, 0x80, 0x03^0x80, 0x9b, 0x61, 0xa1, 0xff, ((0x9b^0x61)^0xa1)^0xff,0x5c};
    unsigned char buffer[11];
    int tam=0,j=0, a=0;
    
    while (buf3[j]!=NULL){
        if(j!=0 || j!=10){
            if(strcmp(buf3[j],F)==0)
            {
                buffer[a]=ESC_OCT1;
                buffer[a+1]=ESC_OCT2;
                a++;
            }
            else if (strcmp(buf3[j],ESC_OCT1)==0)
            {
                buffer[a]=ESC_OCT1;
                buffer[a+1]=ESC_OCT3;
                a++;
            }
        }
        buffer[a]=buf3[j];
        j++;
        a++;
    }

    res = write(fd,buffer,11);
    for(int i=0; i<11;i++){
        printf("%02X ", buffer[i]);
    }

    state=START;
    //RR
    while (state!=STOPP) {       /* loop for input */
       unsigned char buf4[100];
       res = read(fd,buf4,1);   /* returns after 5 chars have been input */
       // buf[res]=0;               /* so we can printf... */
       printf("%02X \n", buf4[0]);
        switch(state){
            case (START):
                if (buf4[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x ", buf4[0]);
                   
                }
                else {
                    state=START;
                    
                } break;
            case (FLAG_RCV):
                if (buf4[0] == A){
                    state=A_RCV; 
                    printf("A=%02x ", buf4[0]);
                 
                        
                }
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                    
                }
                else {
                    state=START;
                    
                } break;
            case (A_RCV):
                /*if (buf4[0]==SET){
                    state=C_RCV_SET;
                    BCC1=A^SET;
                    
                }
                else if(buf4[0]==DISC){
                    state=C_RCV_DISC;
                    BCC1=A^DISC;
                    
                }
                */
                if(buf4[0]==RR0){
                    state=C_RCV_RR0;
                    BCC1=A^RR0;
                    printf("C=%02x ", buf4[0]);
                
                }
                else if(buf4[0]==RR1){
                    state=C_RCV_RR1;
                    BCC1=A^RR1;
                    printf("C=%02x ", buf4[0]);
                
                } 
                else if(buf4[0]==REJ0){
                    state=C_RCV_REJ0;
                    BCC1=A^REJ0;
                    printf("C=%02x ", buf4[0]);
                 
                
                } 
                else if(buf4[0]==REJ1){
                    state=C_RCV_REJ1;
                    BCC1=A^REJ1;
                    printf("C=%02x ", buf4[0]);
      
                } 
                /*else if(buf4[0]==UA){
                    state=C_RCV_UA;
                    BCC1=A^UA;
                } */ 
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                   
                }   
                else {
                    state=START;
                } break;

            case (C_RCV_RR0):
                if(buf4[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf4[0]);
                    
                }
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case (C_RCV_RR1):
                if(buf4[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf4[0]);
                }
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case (C_RCV_REJ0):
                if(buf4[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf4[0]);
                    
                }
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case (C_RCV_REJ1):
                if(buf4[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x\n", buf4[0]);
                    
                }
                else if(buf4[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case (BCC1_OK):
                if(buf4[0]==F){
                    state=STOPP;    
                }
                else {
                    state=START;
                } break;
        }
    }

    //disc
    unsigned char buf5[5] ={0x5c, 0x03, DISC, 0x03^DISC,0x5c};
    res = write(fd,buf5,5);
    printf("%d ", res);
    for(int i=0; i<5;i++){
        printf("%02X ", buf5[i]);
    }

    state = START;
    //receber disc
    while (state!=STOPP) {       /* loop for input */
        unsigned char buf6[1];
        res = read(fd,buf6,1);   /* returns after 5 chars have been input */
        buf6[res]=0;               /* so we can printf... */
       
        
        switch(state){
            case (START):
                if (buf6[0] == F){
                    state=FLAG_RCV;  
                    printf("F=%02x ", buf6[0]);                 
                }
                else {
                    state=START;
                } break;
            case FLAG_RCV:
                if (buf6[0] == A){
                    state=A_RCV;    
                    printf("A=%02x ", buf6[0]);  
                }
                else if(buf6[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case (A_RCV):
                if (buf6[0]==DISC){
                    state=C_RCV_DISC;
                    printf("C=%02x ", buf6[0]);
                }
                else if(buf6[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case C_RCV_DISC:
                if (buf6[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x\n", buf6[0]);
                }
                else if(buf6[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case (BCC1_OK):
                if(buf6[0]==F){
                    state=STOPP;
                }
                else {
                    state=START;
                } break;
        }
        
    }

    //enviar UA
    unsigned char buf7[5] ={0x5c, 0x03, UA, 0x03^UA,0x5c};
        res = write(fd,buf7,5);
        printf("%d ", res);
        for(int i=0; i<5;i++){
            printf("%02X ", buf7[i]);
        }

    sleep(1);
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

	
    close(fd);
    return 0;
}
