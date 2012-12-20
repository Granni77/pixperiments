#include "pi_hardware.h"
#include "math.c"
#include "uart.c"

#define ERRORMASK (BCM2835_GAPO2 | BCM2835_GAPO1 | \
    BCM2835_RERR1 | BCM2835_WERR1)

#define AUDIO_BUFFER_SZ 128
#define PROCESS_CHUNK_SZ 16
unsigned int samplerate;
unsigned int audio_buffer[AUDIO_BUFFER_SZ];
unsigned int process_buffer[PROCESS_CHUNK_SZ];

struct bcm2708_dma_cb __attribute__((aligned(32))) cb_chain[AUDIO_BUFFER_SZ]; // dma control blocks have to be aligned to 128 bit boundary

typedef struct {
    unsigned int buffer[AUDIO_BUFFER_SZ];
    unsigned int read_p, write_p;
} ringbuffer;

ringbuffer buf;

void ringbuffer_init() {
    buf.read_p = 0;
    buf.write_p = 0;
    unsigned int i;
    for (i=0;i<AUDIO_BUFFER_SZ;i++) {
        buf.buffer[i]=0;
    }
}

int buffer_hungry() {
    buf.read_p = (struct bcm2708_dma_cb*)GET32(DMA5_CNTL_BASE + DMA_CNTL_CONBLK_AD) - cb_chain;
    unsigned int spare;
    if (buf.read_p >= buf.write_p) {
        spare = buf.read_p - buf.write_p;
    } else {
        spare = AUDIO_BUFFER_SZ - (buf.write_p - buf.read_p);
    }
    return (spare >= PROCESS_CHUNK_SZ);
}


void audio_init(void) {
    ringbuffer_init();
    // Set up the pwm clock
    // vals read from raspbian:
    // PWMCLK_CNTL = 148 = 10010100 - 100 is allegedly 'plla' but I can't make that work
    // PWMCLK_DIV = 16384
    // PWM_CONTROL=9509 = 10010100100101
    // PWM0_RANGE=1024
    // PWM1_RANGE=1024
    unsigned int range = 0x400;
    unsigned int idiv = 2; // 1 seems to fail
    SET_GPIO_ALT(40, 0); // set pins 40/45 (aka phone jack) to pwm function
    SET_GPIO_ALT(45, 0);
    pause(10); // I don't know if all these pauses are really necessary

    PUT32(CLOCK_BASE + 4*BCM2835_PWMCLK_CNTL, PM_PASSWORD | BCM2835_PWMCLK_CNTL_KILL);
    PUT32(PWM_BASE + 4*BCM2835_PWM_CONTROL, 0);

    samplerate = 19200000 / range / idiv;
    PUT32(CLOCK_BASE + 4*BCM2835_PWMCLK_DIV, PM_PASSWORD | (idiv<<12));
    
    PUT32(CLOCK_BASE + 4*BCM2835_PWMCLK_CNTL,
                                PM_PASSWORD | 
                                BCM2835_PWMCLK_CNTL_ENABLE |
                                BCM2835_PWMCLK_CNTL_OSCILLATOR);
    pause(1);
    PUT32(PWM_BASE + 4*BCM2835_PWM0_RANGE, range);
    PUT32(PWM_BASE + 4*BCM2835_PWM1_RANGE, range);
    pause(1);

    unsigned int i;
    for (i=0;i<AUDIO_BUFFER_SZ;i++) {
        cb_chain[i].info   = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP | BCM2708_DMA_D_DREQ | BCM2708_DMA_PER_MAP(5);
        cb_chain[i].src = (unsigned int)(buf.buffer+i);
        cb_chain[i].dst = ((PWM_BASE+4*BCM2835_PWM_FIFO) & 0x00ffffff) | 0x7e000000; // physical address of fifo
        cb_chain[i].length = sizeof(unsigned int);
        cb_chain[i].stride = 0;
        cb_chain[i].next = (unsigned int)&cb_chain[i+1];
    }
    cb_chain[i-1].next = (unsigned int)&cb_chain[0];
    pause(1);
    PUT32(PWM_BASE + 4*BCM2835_PWM_DMAC, PWMDMAC_ENAB | PWMDMAC_THRSHLD);
    pause(1);
    PUT32(PWM_BASE + 4*BCM2835_PWM_CONTROL, PWMCTL_CLRF);
    pause(1);

    PUT32(PWM_BASE + 4*BCM2835_PWM_CONTROL,
          BCM2835_PWM1_USEFIFO | 
//          BCM2835_PWM1_REPEATFF |
          BCM2835_PWM1_ENABLE | 
          BCM2835_PWM0_USEFIFO | 
//          BCM2835_PWM0_REPEATFF |
          BCM2835_PWM0_ENABLE | 1<<6);


    PUT32(DMA5_CNTL_BASE + DMA_CNTL_CS, BCM2708_DMA_RESET);
    pause(10);
    PUT32(DMA5_CNTL_BASE + DMA_CNTL_CS, BCM2708_DMA_INT | BCM2708_DMA_END);
    PUT32(DMA5_CNTL_BASE + DMA_CNTL_CONBLK_AD, (unsigned int)&cb_chain[0]);
    PUT32(DMA5_CNTL_BASE + DMA_CNTL_DEBUG, 7); // clear debug error flags
    pause(10);
    PUT32(DMA5_CNTL_BASE + DMA_CNTL_CS, 0x10880001);  // go, mid priority, wait for outstanding writes

//    uart_print("audio init done\r\n");
    pause(1);
}
