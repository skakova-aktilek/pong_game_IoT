#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>   // for itoa()
#include "ST7735.h"
#include "LCD_GFX.h"
#include <stdio.h>

/* ==========================================================
   HARDWARE PIN DEFINITIONS
   ========================================================== */
#define LED_P1_PORT   PORTB
#define LED_P1_DDR    DDRB
#define LED_P1_PIN    PB4      // Player 1 LED

#define LED_P2_PORT   PORTD
#define LED_P2_DDR    DDRD
#define LED_P2_PIN    PD7      // Player 2 LED

#define BUZZER_DDR    DDRD
#define BUZZER_PIN    PD5      // OC0B ? buzzer PWM

#define MOTOR_DDR     DDRD
#define MOTOR_PIN     PD6      // OC0A ? DC motor PWM 

#define JOY_CHANNEL   1        // ADC1 (PC1 ? joystick vertical axis)

/* ==========================================================
   GAME SETTINGS
   ========================================================== */
#define BG            rgb565(0, 0, 40)
#define FG            rgb565(255,255,255)
#define BALL_COLOR    rgb565(255, 0, 0)
#define NET_COLOR     rgb565(80, 80, 120)

#define PADDLE_W      4
#define PADDLE_H      20
#define BALL_R        3

#define P1_X          2
#define P2_X          (LCD_WIDTH - PADDLE_W - 2)

#define MAX_SCORE     5
#define FRAME_MS      25

/* ==========================================================
   GLOBAL VARIABLES
   ========================================================== */
static int16_t p1_y, p2_y;
static int16_t ball_x, ball_y;
static int8_t  vx, vy;
static uint8_t score1 = 0, score2 = 0;

/* ==========================================================
   INITIALIZATION FUNCTIONS
   ========================================================== */
void leds_init(void) {
    LED_P1_DDR |= (1<<LED_P1_PIN);
    LED_P2_DDR |= (1<<LED_P2_PIN);
    LED_P1_PORT &= ~(1<<LED_P1_PIN);
    LED_P2_PORT &= ~(1<<LED_P2_PIN);
}

void pwm_init(void) {
    BUZZER_DDR |= (1<<BUZZER_PIN);
    MOTOR_DDR  |= (1<<MOTOR_PIN);
    TCCR0A = 0;
    TCCR0B = 0;
    OCR0A = 0;
    OCR0B = 0;
}

void adc_init(void) {
    ADMUX  = (1<<REFS0);  // AVcc reference
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // clk/128
}

uint16_t adc_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1<<ADSC);
    while (ADCSRA & (1<<ADSC));
    return ADC;
}

/* ==========================================================
   HELPER FUNCTIONS
   ========================================================== */
int16_t clamp16(int16_t v, int16_t lo, int16_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms) {
    if (freq_hz == 0) return;

    // Configure Timer0 for square wave on OC0B (PD5)
    DDRD |= (1 << BUZZER_PIN);
    TCCR0A = (1 << COM0B0) | (1 << WGM01);   // Toggle OC0B on Compare Match, CTC mode
    TCCR0B = (1 << CS01);                    // prescaler = 8

    // Compute TOP value for desired frequency
    // f_OC0B = F_CPU / (2 * N * (1 + OCR0A))
    uint32_t top = (F_CPU / (2UL * 8UL * freq_hz)) - 1;
    if (top > 255) top = 255;                // 8-bit limit
    OCR0A = (uint8_t)top;

    // Duration loop
    for (uint16_t i = 0; i < duration_ms; i++)
        _delay_ms(1);

    // Turn off tone
    TCCR0A = 0;
    TCCR0B = 0;
    PORTD &= ~(1 << BUZZER_PIN);
}

/* --- Motor Animation (Commented Out for Safety) ---
void motor_ramp(uint8_t up_ms, uint8_t down_ms) {
    for (uint8_t i = 0; i < 255; i += 4) { OCR0A = i; _delay_ms(up_ms); }
    for (int16_t i = 255; i >= 0; i -= 4) { OCR0A = (uint8_t)i; _delay_ms(down_ms); }
    OCR0A = 0;
}
---------------------------------------------------- */

/* ==========================================================
   DRAWING FUNCTIONS
   ========================================================== */
void draw_net(void) {
    for (uint8_t y = 0; y < LCD_HEIGHT; y += 6)
        LCD_drawBlock(LCD_WIDTH/2 - 1, y, LCD_WIDTH/2 + 1, y+3, NET_COLOR);
}

void draw_scores(void) {
    char buf[8];
    // Left score
    sprintf(buf, "%d", score1);
    LCD_drawString(8, 4, buf, FG, BG);

    sprintf(buf, "%d", score2);
    LCD_drawString(LCD_WIDTH - 20, 4, buf, FG, BG);
}

void draw_paddles_ball(void) {
    LCD_drawBlock(P1_X, p1_y, P1_X + PADDLE_W - 1, p1_y + PADDLE_H - 1, FG);
    LCD_drawBlock(P2_X, p2_y, P2_X + PADDLE_W - 1, p2_y + PADDLE_H - 1, FG);
    LCD_drawCircle(ball_x, ball_y, BALL_R, BALL_COLOR);
}

/* ==========================================================
   GAME LOGIC
   ========================================================== */
void reset_round(int8_t dir) {
    ball_x = LCD_WIDTH / 2;
    ball_y = LCD_HEIGHT / 2;
    vx = dir * 2;
    vy = (rand() & 1) ? 1 : -1;
}

void point_scored(uint8_t player) {
    if (player == 1) {
        score1++;
        LED_P1_PORT |= (1<<LED_P1_PIN);
        buzzer_beep(120, 128);
        LED_P1_PORT &= ~(1<<LED_P1_PIN);
        /* if (score1 >= MAX_SCORE) motor_ramp(2,2); */  // commented out
    } else {
        score2++;
        LED_P2_PORT |= (1<<LED_P2_PIN);
        buzzer_beep(120, 180);
        LED_P2_PORT &= ~(1<<LED_P2_PIN);
        /* if (score2 >= MAX_SCORE) motor_ramp(2,2); */  // commented out
    }
}

void update_ai(void) {
    static int8_t direction = 1;   // 1 = moving down, -1 = moving up
    static uint8_t aiSpeed = 2;    // constant speed

    p2_y += direction * aiSpeed;

    // Bounce off top and bottom edges
    if (p2_y <= 0) {
        p2_y = 0;
        direction = 1;   // start moving down
    } 
    else if (p2_y >= LCD_HEIGHT - PADDLE_H) {
        p2_y = LCD_HEIGHT - PADDLE_H;
        direction = -1;  // start moving up
    }
}


void update_player_from_joystick(void) {
    uint16_t y = adc_read(JOY_CHANNEL);
    if (y > 700) p1_y -= 3;
    else if (y < 300) p1_y += 3;
    p1_y = clamp16(p1_y, 0, LCD_HEIGHT - PADDLE_H);
}

void update_ball(void) {
    ball_x += vx;
    ball_y += vy;

    // Bounce top/bottom
    if (ball_y - BALL_R <= 0) { ball_y = BALL_R; vy = -vy; buzzer_beep(20, 80); }
    if (ball_y + BALL_R >= LCD_HEIGHT-1) { ball_y = LCD_HEIGHT-1-BALL_R; vy = -vy; buzzer_beep(20, 80); }

    // Paddle collisions
    if (ball_x - BALL_R <= P1_X + PADDLE_W) {
        if (ball_y >= p1_y && ball_y <= p1_y + PADDLE_H) { vx = -vx; buzzer_beep(20, 100); }
        else if (ball_x < 0) { point_scored(2); reset_round(+1); }
    }

    if (ball_x + BALL_R >= P2_X) {
        if (ball_y >= p2_y && ball_y <= p2_y + PADDLE_H) { vx = -vx; buzzer_beep(20, 100); }
        else if (ball_x > LCD_WIDTH-1) { point_scored(1); reset_round(-1); }
    }
}

/* ==========================================================
   MAIN LOOP
   ========================================================== */
int main(void)
{
   
    leds_init();
    pwm_init();
    adc_init();
    lcd_init();

    LCD_setScreen(BG);

    p1_y = LCD_HEIGHT/2 - PADDLE_H/2;
    p2_y = LCD_HEIGHT/2 - PADDLE_H/2;
    reset_round((rand() & 1) ? 1 : -1);

    LCD_drawString(20, 60, "PONG READY", FG, BG);
    _delay_ms(800);

    while (1) {
        update_player_from_joystick();
        update_ai();
        update_ball();

        LCD_setScreen(BG);
        draw_net();
        draw_scores();
        draw_paddles_ball();

        if (score1 >= MAX_SCORE || score2 >= MAX_SCORE) {
            LCD_drawString(20, 80, "GAME OVER", FG, BG);
            _delay_ms(1200);
            score1 = score2 = 0;
            reset_round((rand() & 1) ? 1 : -1);
        }

        _delay_ms(FRAME_MS);
    }
}


