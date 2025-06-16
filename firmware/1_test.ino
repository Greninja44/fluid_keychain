#include <FastLED.h>
#include <Wire.h>
#include <MPU6050.h>   // dummy sensor for now
//#include "SensorQMI8658.hpp" // actual sensor for Waveshare ESP32-S3-Matrix

// Pin definitions - basic hardware setup
#define LED_PIN     14
#define SDA_PIN     11
#define SCL_PIN     12
#define BUTTON_PIN  4   // color switching button
#define MODE_BUTTON_PIN 5

#define NUM_LEDS    64
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define FLUID_PARTICLES 30  // tweak this if system lags
#define BRIGHTNESS  10
#define NUM_COLORS  8   // RGB gang! 🌈

// Basic data structures for 2D physics
struct Vector2D {
    float x;
    float y;
};

struct Particle {
    Vector2D position;
    Vector2D velocity;
};

// Global variables - all the important stuff lives here
CRGB leds[NUM_LEDS];
SensorQMI8658 mpu;
IMUdata Accel;
IMUdata Gyro;
Particle particles[FLUID_PARTICLES];
Vector2D acceleration = {0, 0};

// Button and mode control variables
uint8_t currentColorIndex = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;  // anti-spam protection
uint8_t currentMode = 0;

// Color palette - HSV values for different vibes
const uint8_t COLORS[NUM_COLORS] = {
    64,   // Yellow - traffic light vibes
    160,  // Blue - metro line colors
    0,    // Red - classic choice
    32,   // Orange - sunset feels
    96,   // Green - nature mode
    128,  // Aqua - chill vibes
    192,  // Purple - aesthetic mode
    224,  // Pink - millennial approved
};

// Thread safety stuff - prevents crashes when multiple tasks access data
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;

// Physics constants - Newton would be proud
const float GRAVITY = 0.08f;
const float DAMPING = 0.92f;
const float MAX_VELOCITY = 2.9f;

// Function prototypes - declaring what's coming up
void initMPU();
void initLEDs();
void initParticles();
void updateParticles();
void drawParticles();
void MPUTask(void *parameter);
void LEDTask(void *parameter);
void checkButton();
void drawText();

// Binary art data - each pattern is 8x8 pixels encoded as binary
// This creates scrolling text animation on the LED matrix
const uint8_t IMAGES[][8] = {
{
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
},{
  0b11000011,
  0b11000011,
  0b11000011,
  0b11111111,
  0b11111111,
  0b11000011,
  0b11000011,
  0b11000011
},{
},
  0b10000110,
// ... more patterns here (truncated for brevity)
  0b10000110,
{
  0b10000110,
  0b11111110,
  0b11111110,
  0b10000110,
  0b10000110,
  0b10000110
},{
  0b00001100,
  0b00001100,
  0b00001100,
  0b11111100,
  0b11111100,
  0b00001100,
  0b00001100,
  0b00001100
},{
  0b00011000,
  0b00011000,
  0b00011000,
  0b11111000,
  0b11111000,
  0b00011000,
  0b00011000,
  0b00011000
},{
  0b00110001,
  0b00110001,
  0b00110001,
  0b11110001,
  0b11110001,
  0b00110001,
  0b00110001,
  0b00110001
},{
  0b01100011,
  0b01100011,
  0b01100011,
  0b11100011,
  0b11100011,
  0b01100011,
  0b01100011,
  0b01100011
},{
  0b11000111,
  0b11000111,
  0b11000110,
  0b11000111,
  0b11000111,
  0b11000110,
  0b11000111,
  0b11000111
},{
  0b10001111,
  0b10001111,
  0b10001100,
  0b10001111,
  0b10001111,
  0b10001100,
  0b10001111,
  0b10001111
},{
  0b00011111,
  0b00011111,
  0b00011000,
  0b00011111,
  0b00011111,
  0b00011000,
  0b00011111,
  0b00011111
},{
  0b00111111,
  0b00111111,
  0b00110000,
  0b00111111,
  0b00111111,
  0b00110000,
  0b00111111,
  0b00111111
},{
  0b01111110,
  0b01111110,
  0b01100000,
  0b01111110,
  0b01111110,
  0b01100000,
  0b01111110,
  0b01111110
},{
  0b11111100,
  0b11111100,
  0b11000000,
  0b11111100,
  0b11111100,
  0b11000000,
  0b11111100,
  0b11111100
},{
  0b11111000,
  0b11111000,
  0b10000000,
  0b11111000,
  0b11111000,
  0b10000000,
  0b11111000,
  0b11111000
},{
  0b11110001,
  0b11110001,
  0b00000001,
  0b11110001,
  0b11110001,
  0b00000001,
  0b11110001,
  0b11110001
},{
  0b11100011,
  0b11100011,
  0b00000011,
  0b11100011,
  0b11100011,
  0b00000011,
  0b11100011,
  0b11100011
},{
  0b11000110,
  0b11000110,
  0b00000110,
  0b11000110,
  0b11000110,
  0b00000110,
  0b11000111,
  0b11000111
},{
  0b10001100,
  0b10001100,
  0b00001100,
  0b10001100,
  0b10001100,
  0b00001100,
  0b10001111,
  0b10001111
},{
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011111,
  0b00011111
},{
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00111111,
  0b00111111
},{
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01111110,
  0b01111110
},{
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11111100,
  0b11111100
},{
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b11111000,
  0b11111000
},{
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b11110001,
  0b11110001
},{
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b11100011,
  0b11100011
},{
  0b00000110,
  0b00000110,
  0b00000110,
  0b00000110,
  0b00000110,
  0b00000110,
  0b11000111,
  0b11000111
},{
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b10001111,
  0b10001111
},{
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011111,
  0b00011111
},{
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00111111,
  0b00111111
},{
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01111110,
  0b01111110
},{
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11000000,
  0b11111100,
  0b11111100
},{
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b10000000,
  0b11111000,
  0b11111000
},{
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b11110001,
  0b11110001
},{
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b00000011,
  0b11100011,
  0b11100011
},{
  0b00000111,
  0b00000111,
  0b00000110,
  0b00000110,
  0b00000110,
  0b00000110,
  0b11000111,
  0b11000111
},{
  0b00001111,
  0b00001111,
  0b00001100,
  0b00001100,
  0b00001100,
  0b00001100,
  0b10001111,
  0b10001111
},{
  0b00011111,
  0b00011111,
  0b00011001,
  0b00011001,
  0b00011001,
  0b00011001,
  0b00011111,
  0b00011111
},{
  0b00111111,
  0b00111111,
  0b00110011,
  0b00110011,
  0b00110011,
  0b00110011,
  0b00111111,
  0b00111111
},{
  0b01111110,
  0b01111110,
  0b01100110,
  0b01100110,
  0b01100110,
  0b01100110,
  0b01111110,
  0b01111110
},{
  0b11111100,
  0b11111100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11111100,
  0b11111100
},{
  0b11111000,
  0b11111000,
  0b10011000,
  0b10011000,
  0b10011000,
  0b10011000,
  0b11111000,
  0b11111000
},{
  0b11110000,
  0b11110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b00110000,
  0b11110000,
  0b11110000
},{
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
},{
{
  0b10000001,
  0b10011001,
  0b10011001,
  0b11011011,
  0b11011011,
  0b11111111,
  0b01100110,
  0b01000010
}};
const int IMAGES_LEN = sizeof(IMAGES)/8;

// Converts 2D coordinates to 1D LED array index with 90-degree rotation
int xy(int x, int y) {
    x = constrain(x, 0, MATRIX_WIDTH - 1);
    y = constrain(y, 0, MATRIX_HEIGHT - 1);
    
    int newX = MATRIX_HEIGHT - 1 - y;
    int newY = x;
    
    return newY * MATRIX_WIDTH + newX;
}

// Button debouncing for color changes - prevents accidental multiple presses
void checkButton() {
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
            currentColorIndex = (currentColorIndex + 1) % NUM_COLORS;
            lastDebounceTime = millis();
        }
    }
    lastButtonState = buttonState;
}

// Mode switching between particle physics and text animation
void checkModeButton(){
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(MODE_BUTTON_PIN);

    if (buttonState == LOW && lastButtonState == HIGH) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
            currentMode = (currentMode + 1) % 2;
            lastDebounceTime = millis();
        }
    }
    lastButtonState = buttonState;
}

// Displays scrolling text animation using binary patterns
// Each frame shows for 175ms creating smooth animation
void drawText() {
    static uint8_t currentImageIndex = 0;
    static unsigned long lastImageChangeTime = 0;
    const unsigned long imageChangeDelay = 175;

    FastLED.clear();

    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        uint8_t rowByte = IMAGES[currentImageIndex][y];
        
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            if ((rowByte >> (MATRIX_WIDTH - 1 - x)) & 0x01) {
                leds[xy(x, y)] = CHSV(COLORS[currentColorIndex], 255, 255);
            }
        }
    }
    FastLED.show();

    if (millis() - lastImageChangeTime > imageChangeDelay) {
        currentImageIndex = (currentImageIndex + 1) % IMAGES_LEN;
        lastImageChangeTime = millis();
    }
}

// Main particle rendering function with collision detection
// Sorts particles by position, handles overlapping, and adds speed-based brightness
void drawParticles() {
    FastLED.clear();
    
    bool occupied[MATRIX_WIDTH][MATRIX_HEIGHT] = {{false}};
    
    struct ParticleIndex {
        int index;
        float position;
    };
    
    // Sort particles by position for better rendering
    ParticleIndex sortedParticles[FLUID_PARTICLES];
    for (int i = 0; i < FLUID_PARTICLES; i++) {
        sortedParticles[i].index = i;
        sortedParticles[i].position = particles[i].position.y * MATRIX_WIDTH + particles[i].position.x;
    }
    
    // Bubble sort - simple and reliable
    for (int i = 0; i < FLUID_PARTICLES - 1; i++) {
        for (int j = 0; j < FLUID_PARTICLES - i - 1; j++) {
            if (sortedParticles[j].position > sortedParticles[j + 1].position) {
                ParticleIndex temp = sortedParticles[j];
                sortedParticles[j] = sortedParticles[j + 1];
                sortedParticles[j + 1] = temp;
            }
        }
    }

    // Render each particle with collision avoidance
    for (int i = 0; i < FLUID_PARTICLES; i++) {
        int particleIndex = sortedParticles[i].index;
        int x = round(particles[particleIndex].position.x);
        int y = round(particles[particleIndex].position.y);
        
        x = constrain(x, 0, MATRIX_WIDTH - 1);
        y = constrain(y, 0, MATRIX_HEIGHT - 1);
        
        if (!occupied[x][y]) {
            int index = xy(x, y);
            if (index >= 0 && index < NUM_LEDS) {
                // Brightness based on particle speed - faster = brighter
                float speed = sqrt(
                    particles[particleIndex].velocity.x * particles[particleIndex].velocity.x + 
                    particles[particleIndex].velocity.y * particles[particleIndex].velocity.y
                );
                
                uint8_t hue = COLORS[currentColorIndex];
                uint8_t sat = 255;
                uint8_t val = constrain(180 + (speed * 50), 180, 255);
                
                leds[index] = CHSV(hue, sat, val);
                occupied[x][y] = true;
            }
        } else {
            // Find nearby free spot if position is occupied
            for (int r = 1; r < 3; r++) {
                for (int dx = -r; dx <= r; dx++) {
                    for (int dy = -r; dy <= r; dy++) {
                        if (abs(dx) + abs(dy) == r) {
                            int newX = x + dx;
                            int newY = y + dy;
                            if (newX >= 0 && newX < MATRIX_WIDTH && 
                                newY >= 0 && newY < MATRIX_HEIGHT && 
                                !occupied[newX][newY]) {
                                int index = xy(newX, newY);
                                if (index >= 0 && index < NUM_LEDS) {
                                    leds[index] = CHSV(COLORS[currentColorIndex], 255, 180);
                                    occupied[newX][newY] = true;
                                    goto nextParticle;
                                }
                            }
                        }
                    }
                }
            }
            nextParticle:
            continue;
        }
    }
    
    FastLED.show();
}

// Physics simulation - handles gravity, collisions, and particle interactions
// Each particle responds to accelerometer input and bounces off walls
void updateParticles() {
    Vector2D currentAccel;
    portENTER_CRITICAL(&dataMux);
    currentAccel = acceleration;
    portEXIT_CRITICAL(&dataMux);

    currentAccel.x *= 0.3f;
    currentAccel.y *= 0.3f;

    // Update particle physics
    for (int i = 0; i < FLUID_PARTICLES; i++) {
        particles[i].velocity.x = particles[i].velocity.x * 0.9f + (currentAccel.x * GRAVITY);
        particles[i].velocity.y = particles[i].velocity.y * 0.9f + (currentAccel.y * GRAVITY);

        particles[i].velocity.x = constrain(particles[i].velocity.x, -MAX_VELOCITY, MAX_VELOCITY);
        particles[i].velocity.y = constrain(particles[i].velocity.y, -MAX_VELOCITY, MAX_VELOCITY);

        float newX = particles[i].position.x + particles[i].velocity.x;
        float newY = particles[i].position.y + particles[i].velocity.y;

        // Wall collision detection and bounce
        if (newX < 0.0f) {
            newX = 0.0f;
            particles[i].velocity.x = fabs(particles[i].velocity.x) * DAMPING;
        } 
        else if (newX >= (MATRIX_WIDTH - 1)) {
            newX = MATRIX_WIDTH - 1;
            particles[i].velocity.x = -fabs(particles[i].velocity.x) * DAMPING;
        }

        if (newY < 0.0f) {
            newY = 0.0f;
            particles[i].velocity.y = fabs(particles[i].velocity.y) * DAMPING;
        } 
        else if (newY >= (MATRIX_HEIGHT - 1)) {
            newY = MATRIX_HEIGHT - 1;
            particles[i].velocity.y = -fabs(particles[i].velocity.y) * DAMPING;
        }

        particles[i].position.x = constrain(newX, 0.0f, MATRIX_WIDTH - 1);
        particles[i].position.y = constrain(newY, 0.0f, MATRIX_HEIGHT - 1);

        particles[i].velocity.x *= 0.95f;
        particles[i].velocity.y *= 0.95f;
    }

    // Particle-to-particle repulsion - prevents clustering
    for (int i = 0; i < FLUID_PARTICLES; i++) {
        for (int j = i + 1; j < FLUID_PARTICLES; j++) {
            float dx = particles[j].position.x - particles[i].position.x;
            float dy = particles[j].position.y - particles[i].position.y;
            float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < 1.0f) {
                float distance = sqrt(distanceSquared);
                float angle = atan2(dy, dx);
                
                float repulsionX = cos(angle) * 0.5f;
                float repulsionY = sin(angle) * 0.5f;

                particles[i].position.x -= repulsionX * 0.3f;
                particles[i].position.y -= repulsionY * 0.3f;
                particles[j].position.x += repulsionX * 0.3f;
                particles[j].position.y += repulsionY * 0.3f;

                Vector2D avgVel = {
                    (particles[i].velocity.x + particles[j].velocity.x) * 0.5f,
                    (particles[i].velocity.y + particles[j].velocity.y) * 0.5f
                };

                particles[i].velocity = avgVel;
                particles[j].velocity = avgVel;
            }
        }
    }
}

// Sensor initialization with error handling
void initMPU() {
    Serial.println("Initializing QMI8658...");
    if (!mpu.begin(Wire, QMI8658_L_SLAVE_ADDRESS, SDA_PIN, SCL_PIN)) {
        Serial.println("IMU not found. Check wiring!");
        while (1) delay(1000);
    }

    // Configure accelerometer and gyroscope settings
    mpu.configAccelerometer(SensorQMI8658::ACC_RANGE_4G,
                            SensorQMI8658::ACC_ODR_1000Hz,
                            SensorQMI8658::LPF_MODE_0);

    mpu.configGyroscope(SensorQMI8658::GYR_RANGE_64DPS,
                        SensorQMI8658::GYR_ODR_896_8Hz,
                        SensorQMI8658::LPF_MODE_3);

    mpu.enableGyroscope();
    mpu.enableAccelerometer();
    Serial.println("IMU initialized.");
}

// LED strip setup with brightness control
void initLEDs() {
    Serial.println("Initializing LEDs...");
    FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear(true);
    Serial.println("LEDs initialized");
}

// Initialize particles in bottom rows of matrix
void initParticles() {
    Serial.println("Initializing particles...");
    int index = 0;
    
    for (int y = MATRIX_HEIGHT - 4; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH && index < FLUID_PARTICLES; x++) {
            particles[index].position = {static_cast<float>(x), static_cast<float>(y)};
            particles[index].velocity = {0.0f, 0.0f};
            index++;
        }
    }
    
    Serial.printf("Total particles initialized: %d\n", index);
}

// Background task for reading sensor data - runs on core 0
void MPUTask(void *parameter) {
    while (true) {
        float ax, ay, az;
        if (mpu.getDataReady()) {
            mpu.getAccelerometer(Accel.x, Accel.y, Accel.z);
            ax = Accel.x;
            ay = Accel.y;
            az = Accel.z;
        }
        
        // Thread-safe update of acceleration data
        portENTER_CRITICAL(&dataMux);
        acceleration.x = ax;
        acceleration.y = ay;
        portEXIT_CRITICAL(&dataMux);
        
        printf("ACCEL: %f %f\r\n", acceleration.x, acceleration.y);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Main display task - handles buttons and rendering - runs on core 1
void LEDTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(16);  // ~60 FPS
    
    while (true) {
        checkButton();
        checkModeButton();

        switch (currentMode) {
            case 0:
                updateParticles();
                drawParticles();
                break;
            
            case 1:
                drawText();
                break;
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Main setup function - initializes everything and creates tasks
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting initialization...");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    initMPU();
    initLEDs();
    initParticles();

    // Create dual-core tasks for better performance
    xTaskCreatePinnedToCore(MPUTask, "MPUTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(LEDTask, "LEDTask", 4096, NULL, 1, NULL, 1);

    Serial.println("Setup complete - ready to rock! 🚀");
}

void loop() {
    vTaskDelete(NULL);  // Main loop not needed, tasks handle everything
}
*const uint8_t IMAGES[][8] = {
{
  0b00011000,
  0b00111100,
  0b01100110,
  0b11000011,
  0b11111111,
  0b11111111,
  0b11000011,
  0b11000011
},{
  0b11111100,
  0b11000110,
  0b11000110,
  0b11111100,
  0b11111100,
  0b11000110,
  0b11000110,
  0b11111100
},{
  0b01111111,
  0b11111111,
  0b11100000,
  0b11000000,
  0b11000000,
  0b11100000,
  0b11111111,
  0b01111111
},{
  0b11110000,
  0b11111100,
  0b11001110,
  0b11000110,
  0b11000110,
  0b11001110,
  0b11111100,
  0b11110000
},{
  0b01111110,
  0b01111110,
  0b01100000,
  0b01111110,
  0b01111110,
  0b01100000,
  0b01111110,
  0b01111110
},{
  0b11111111,
  0b11111111,
  0b11000000,
  0b11111100,
  0b11111100,
  0b11000000,
  0b11000000,
  0b11000000
},{
  0b00111110,
  0b01111110,
  0b11000000,
  0b11001110,
  0b11011110,
  0b11011000,
  0b01111000,
  0b00111000
},{
  0b11000011,
  0b11000011,
  0b11000011,
  0b11111111,
  0b11111111,
  0b11000011,
  0b11000011,
  0b11000011
},{
  0b11111111,
  0b11111111,
  0b10011001,
  0b00011000,
  0b00011000,
  0b10011001,
  0b11111111,
  0b11111111
},{
  0b01111110,
  0b01111110,
  0b00000110,
  0b00000110,
  0b00000110,
  0b00001110,
  0b01111100,
  0b01111000
},{
  0b11000110,
  0b11001110,
  0b11011100,
  0b11111000,
  0b11111000,
  0b11011100,
  0b11001110,
  0b11000110
},{
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01100000,
  0b01111110,
  0b01111110
},{
  0b11000011,
  0b11100111,
  0b11111111,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11000011
},{
  0b11000011,
  0b11100011,
  0b11110011,
  0b11011011,
  0b11001111,
  0b11000111,
  0b11000011,
  0b11000011
},{
  0b11111111,
  0b11111111,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11111111,
  0b11111111
},{
  0b01111110,
  0b01111110,
  0b01100110,
  0b01111110,
  0b01111110,
  0b01100000,
  0b01100000,
  0b01100000
},{
  0b11111100,
  0b11111100,
  0b11001100,
  0b11001100,
  0b11111100,
  0b11111100,
  0b00000110,
  0b00000111
},{
  0b11111110,
  0b11111110,
  0b11000110,
  0b11000110,
  0b11111110,
  0b11111100,
  0b11000110,
  0b11000111
},{
  0b11111111,
  0b11111111,
  0b11000000,
  0b11111111,
  0b11111111,
  0b00000011,
  0b11111111,
  0b11111111
},{
  0b11111111,
  0b11111111,
  0b10011001,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000
},{
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b01111110,
  0b01111110
},{
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b01100110,
  0b00111100,
  0b00011000
},{
  0b10000001,
  0b11000011,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11011011,
  0b11111111,
  0b11111111
},{
  0b11000001,
  0b01100011,
  0b00110110,
  0b00011100,
  0b00111000,
  0b01101100,
  0b11000110,
  0b10000011
},{
  0b10000001,
  0b11000011,
  0b01100110,
  0b00111100,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000
},{
  0b11111111,
  0b11111111,
  0b00000011,
  0b00000110,
  0b00001100,
  0b00011000,
  0b01111111,
  0b11111111
}};
const int IMAGES_LEN = sizeof(IMAGES)/8;
                                              // THIS IS THE ALPHABETS IN A LINEAR WAY

