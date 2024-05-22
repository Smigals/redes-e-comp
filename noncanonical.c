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
#define BCC1_OK 4
#define STOPP 5
#define C_RCV_I0 6
#define C_RCV_I1 7
#define DADOS 8
#define BCC2_OK 9
#define F 0x5c
#define A 0x03
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
#define C_RCV_UA 16
#define ESC_OCT1 0x5d
#define ESC_OCT2 0x7d
#define ESC_OCT3 0x7c
#define ESC_OCT4 0x5c


int main(int argc, char** argv)
{
    int fd,c, res, state;
    unsigned char octet;
    struct termios oldtio,newtio;
    //unsigned char buf[255];

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
    //Máquina de estados recetor Open//
    unsigned char BCC1, BCC2;
    int RespostaR=0;
    state = START;
    //LER SET
    while (state!=STOPP) {       /* loop for input */
       unsigned char buf[100];
       res = read(fd,buf,1);   /* returns after 5 chars have been input */
       // buf[res]=0;               /* so we can printf... */
       printf("%02X \n", buf[0]);
        
       switch(state){
            case START:
                if (buf[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x ", buf[0]);
                    fflush(stdout);
                }
                else {
                    state=START;
                    
                } break;
            case FLAG_RCV:
                if (buf[0] == A){
                    state=A_RCV; 
                    printf("A=%02x ", buf[0]);  
                    fflush(stdout);  
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
                    printf("C=%02x ", buf[0]);
                    fflush(stdout);
                    
                }
                /*else if(buf[0]==DISC){
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
                }*/
                else if(buf[0]==F){
                    state=FLAG_RCV;
                   
                }   
                else {
                    state=START;
                } break;

            case (C_RCV_SET /*|| C_RCV_DISC || C_RCV_UA*/):
                if(buf[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf[0]);
                    fflush(stdout);
                }
                else if(buf[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case BCC1_OK:
                if (buf[0]==F){
                    state=STOPP;
                    printf("FLAG=%02x\n", buf[0]);
                    fflush(stdout);
                }
                else {
                    state=START;
                   
                } break;
        }
    }

    //ENVIAR UA
    unsigned char buf2[5]={0x5c, 0x01, 0x06, 0x01^0x06, 0x5c};
    res=write(fd, buf2, 5);
    printf("\n\n%d\n", res);
    for(int i=0; i<5; i++) {
        printf("%02x\n", buf2[i]);
    

    }
    int detecao;
    state=START;
    //LER I
    while (state!=STOPP) {       /* loop for input */
       unsigned char buf3[100];
       res = read(fd,buf3,1);   /* returns after 5 chars have been input */
       // buf[res]=0;               /* so we can printf... */
       printf("%02X \n", buf3[0]);
        
        char armazem;
        switch(state){
            case START:
                if (buf3[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x ", buf3[0]);
                }
                else {
                    state=START;
                    
                } break;
            case FLAG_RCV:
                if (buf3[0] == A){
                    state=A_RCV; 
                    printf("A=%02x ", buf3[0]); 
                }
                else if(buf3[0]==F){
                    state=FLAG_RCV;
                    
                }
                else {
                    state=START;
                    
                } break;
            case A_RCV:
                /*if (buf3[0]==SET){
                    state=C_RCV_SET;
                    BCC1=A^SET;
                    
                }
                else if(buf3[0]==DISC){
                    state=C_RCV_DISC;
                    BCC1=A^DISC;
                    
                }
                */
                if(buf3[0]==I0){
                    state=C_RCV_I0;
                    BCC1=A^I0;
                    RespostaR=1;
                    printf("C=%02x ", buf3[0]);

                }
                else if(buf3[0]==I1){
                    state=C_RCV_I1;
                    BCC1=A^I1;
                    RespostaR=0;
                    printf("C=%02x ", buf3[0]);
                } 
                /*else if(buf3[0]==UA){
                    state=C_RCV_UA;
                    BCC1=A^UA;
                } */ 
                else if(buf3[0]==F){
                    state=FLAG_RCV;
                   
                }   
                else {
                    state=START;
                } break;

            case (C_RCV_I0):
                if(buf3[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x ", buf3[0]);
                }
                else if(buf3[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case (C_RCV_I1):
                if(buf3[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC1=%02x\n", buf3[0]);
                }
                else if(buf3[0]==F){
                    state=FLAG_RCV;
                }               
                else {
                    state=START;
          
                } break;
            case BCC1_OK:
                if (buf3[0]==ESC_OCT1){
                    state = BCC1_OK;
                    detecao=1;
                }
                else if (buf3[0]==ESC_OCT3 && detecao == 1)
                {
                    BCC2=BCC2^ESC_OCT4;
                    state = BCC1_OK;
                    detecao=0;
                }
                else if (buf3[0]==ESC_OCT2 && detecao == 1)
                {
                    BCC2=BCC2^ESC_OCT1;
                    state = BCC1_OK;
                    detecao=0;
                }
                else if(buf3[0]==BCC2){
                    state=BCC2_OK;
                    printf("BCC2=%02x\n", buf3[0]);
                }
                else if(buf3[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    octet = buf3[0];
                    BCC2=octet^BCC2;
                    state=BCC1_OK;
                    //fazer xores com valores anteriores
                } break;
            case BCC2_OK:
                if (buf3[0]==F)
                {
                    state = STOPP;
                }
                else{
                    state=START;
                }
        }
    }
    char controloResposta=0;
    if (RespostaR==1)
    {
        controloResposta=RR1;
    }
    else if (RespostaR==0)
    {
        controloResposta=RR0;
    }
    
    //ENVIAR RRs
    unsigned char buf4[5]={0x5c, 0x01, controloResposta, 0x01^controloResposta, 0x5c};
    res=write(fd, buf4, 5);
    printf("\n\n%d\n", res);
    for(int i=0; i<5; i++) {
    printf("%02x\n", buf4[i]);
    }
    state = START;
    //RECEBER DISC
    while (state!=STOPP) {       /* loop for input */
        unsigned char buf5[1];
        res = read(fd,buf5,1);   /* returns after 5 chars have been input */
        buf5[res]=0;               /* so we can printf... */
       
        
        switch(state){
            case START:
                if (buf5[0] == F){
                    state=FLAG_RCV;  
                    printf("F=%02x\n", buf5[0]);                 
                }
                else {
                    state=START;
                } break;
            case FLAG_RCV:
                if (buf5[0] == A){
                    state=A_RCV;    
                    printf("A=%02x\n", buf5[0]);  
                }
                else if(buf5[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case A_RCV:
                if (buf5[0]==DISC){
                    state=C_RCV_DISC;
                    printf("C=%02x\n", buf5[0]);
                }
                else if(buf5[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case C_RCV_DISC:
                if (buf5[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x\n", buf5[0]);
                }
                else if(buf5[0]==F){
                    state=FLAG_RCV;
                    printf("FLAG=%02x\n", buf5[0]);
                }
                else {
                    state=START;
                } break;
            case BCC1_OK:
                if(buf5[0]==F){
                    state=STOPP;
                }
                else {
                    state=START;
                } break;
        }
        
    }
    //ENVIAR DISC
     unsigned char buf6[5] ={0x5c, 0x01, DISC, 0x01^DISC,0x5c};
    res = write(fd,buf6,5);
    printf("%d ", res);
    for(int i=0; i<5;i++){
        printf("%02X ", buf6[i]);
    }

    //LER UA
    state = START;
    while (state!=STOPP) {       /* loop for input */
      unsigned char buf7[1];
       res = read(fd,buf7,1);   /* returns after 5 chars have been input */
       buf7[res]=0;               /* so we can printf... */
       
        
       switch(state){
            case START:
                if (buf7[0] == F){
                    state=FLAG_RCV;
                    printf("F=%02x\n", buf7[0]);
                }
                else {
                    state=START;
                } break;
            case FLAG_RCV:
                if (buf7[0] == A){
                    state=A_RCV; 
                    printf("A=%02x\n", buf7[0]);
                }
                else if(buf2[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case A_RCV:
                if (buf7[0]==UA){
                    state=C_RCV;
                    printf("C=%02x\n", buf7[0]);
                }
                else if(buf7[0]==F){
                    state=FLAG_RCV;
                }
                else {
                    state=START;
                } break;
            case C_RCV:
                if (buf7[0]==BCC1){
                    state=BCC1_OK;
                    printf("BCC=%02x\n", buf7[0]);
                }
                else if(buf7[0]==F){
                    state=FLAG_RCV;
                    printf("FLAG=%02x\n", buf7[0]);
                }
                else {
                    state=START;
                } break;
            case BCC1_OK:
                if(buf7[0]==F){
                    state=STOPP;
                }
                else {
                    state=START;
                } break;
        }    
    }
    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */
    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
