#include "mbed.h"

#define HIGH    1
#define LOW     0
#define TIME    10
#define BOUNCE  40
#define NROBOTONES  4
#define NROLEDS     4
#define NO      1
#define SI      0

#define TTOSTART    1000
#define TCHECK      100
#define TLEDMAX     1501
#define TLEDBASE    400

#define WAITING 0
#define START   1
#define INGAME  2
#define WIN     3
#define LOSE    4

typedef enum{
    BUTTON_DOWN,    //0
    BUTTON_UP,      //1
    BUTTON_FALLING, //2
    BUTTON_RISING   //3
}_eButtonState;

typedef struct{
    uint8_t estado;
    int32_t timeDown;
    int32_t timeUp;
}_sTeclas;

_sTeclas ourButton[NROBOTONES];

// 0001 , 0010,  0100, 1000
uint16_t mask[]={0x0001,0x0002,0x0004,0x0008};

_eButtonState myButton;

DigitalOut ledHeart(PC_13);

BusOut leds(PB_12,PB_13,PB_14,PB_15);

BusIn botones(PB_6,PB_7,PB_8,PB_9);


Timer miTimer;

int miWait1(int ms, int init) //NO BLOCKING WAIT, init= setea primer lastTime
{
    static int lastTime1;
    int actualTime1;

    actualTime1 = miTimer.read_ms();
    
    if(init)
        lastTime1 = miTimer.read_ms();

    if((actualTime1 - lastTime1) >= ms)
    {
        lastTime1 = actualTime1;
        return 1;
    }
    
    return 0;
}

void startMef(uint8_t indice);
void actuallizaMef(uint8_t indice);

int main(){

    int ledRandom=0, ledTimeRandom=0;
    int bitHeart=0, time40=0, gamemode=WAITING;
    int cont=0, generado = NO;

    miTimer.start();
    srand(time(NULL));
    miWait1(1,1);

    for (uint8_t indice = 0; indice < NROBOTONES; indice++)
    {
        startMef(indice);
    }
    
    while(1){
        
        if (miWait1(TIME, 0))
        {
            if (time40 >= BOUNCE)
            {
                time40 = 0;
                for (uint8_t indice = 0; indice < NROBOTONES; indice++)
                {
                    actuallizaMef(indice);
                    if ((ourButton[indice].timeDown - ourButton[indice].timeUp) >= TTOSTART)
                    {
                        gamemode = START;
                        cont = 0;
                    }
                }
            }
        }
            switch (gamemode)
            {
            case WAITING:
                switch (cont)
                {
                case TCHECK:
                    leds = 0x00;
                    leds = 0x06;
                    break;
                case (TCHECK*2):
                    leds = 0x00;
                    leds = 0x09;
                    cont = 0;
                    break;
                default:
                    break;
                }
                break;
            case START:
                switch (cont)
                {
                case 0:
                case 40:
                case 80:
                    leds = 0x0f;
                    break;
                case 20:
                case 60:
                case 100:
                    leds = 0x00;
                default:
                    if (cont >= 120)
                    {
                        gamemode = INGAME;
                        generado = NO;
                        leds = 0x00;
                        cont=0;
                    }
                    break;
                }
                break;
            case INGAME:
                if (generado == NO)
                {
                    ledRandom = rand() % NROLEDS;
                    ledTimeRandom = (rand() % TLEDMAX)+TLEDBASE;
                    leds = mask[ledRandom];
                    generado = SI;
                }else if ((cont*10) >= ledTimeRandom)
                {
                    leds = 0x00;
                    gamemode = LOSE;
                    generado = NO;
                    cont = 0;
                }else if ((ourButton[ledRandom].estado == BUTTON_DOWN))
                {
                    leds = 0x00;
                    gamemode = WIN;
                    generado = NO;
                    cont = 0;
                }
                break;
            case WIN:
                switch (cont)
                {
                case 0:
                case 100:
                case 200:
                    leds = 0x0f;
                case 50:
                case 150:
                case 250:
                    leds = 0x00;
                    break;
                default:
                    if (cont >= 260)
                    {
                        gamemode = INGAME;
                        cont = 0;
                    }
                    break;
                }
                break;
            case LOSE:
                switch (cont)
                {
                case 0:
                case 100:
                case 200:
                    leds = mask[ledRandom];
                case 50:
                case 150:
                case 250:
                    leds = 0x00;
                    break;
                default:
                    if (cont >= 260)
                    {
                        gamemode = WAITING;
                        cont = 0;
                    }
                    break;
                }
                break;
            default:
                break;
            }

            if(bitHeart >= 50)
            {
                ledHeart =! ledHeart;
                bitHeart = 0;
            }

            cont++;
            time40++;
            bitHeart++;
        }
}

void startMef(uint8_t indice){
    ourButton[indice].estado=BUTTON_UP;
    ourButton[indice].timeDown=0;
    ourButton[indice].timeUp=0;
}

void actuallizaMef(uint8_t indice){

    switch (ourButton[indice].estado)
    {
    case BUTTON_UP:
        if(!(botones.read() & mask[indice]))
            ourButton[indice].estado=BUTTON_FALLING;
    break;
    case BUTTON_FALLING:
        if(!(botones.read() & mask[indice]))
        {
            ourButton[indice].timeDown=miTimer.read_ms();
            ourButton[indice].estado=BUTTON_DOWN;
        }else
            ourButton[indice].estado=BUTTON_UP;
    break;
    case BUTTON_DOWN:
        if(botones.read() & mask[indice])
           ourButton[indice].estado=BUTTON_RISING;
    break;
    case BUTTON_RISING:
        if(botones.read() & mask[indice]){
            ourButton[indice].estado=BUTTON_UP;
            ourButton[indice].timeUp=miTimer.read_ms();
        }else
            ourButton[indice].estado=BUTTON_DOWN;
    break;
    default:
        startMef(indice);
        break;
    }
}