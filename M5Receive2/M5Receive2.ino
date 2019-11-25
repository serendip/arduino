#include <M5StickC.h>
#include <driver/i2s.h>
#include "efontEnableAll.h"
#include "efont.h"
#include "efontM5StickC.h"
 
#include "chirp_connect.h"
#include "credentials.h"
 
#define PIN_CLK  0
#define PIN_DATA 34
 
#define LED_PIN           10      // Pin number for on-board LED
//#define SWITCH_PIN        0      // Pin number for on-board switch
 
#define BUFFER_SIZE       512    // Audio buffer size
#define SAMPLE_RATE       16000  // Audio sample rate
 
 
#define MIC_CALIBRATION   13125
#define CONVERT_INPUT(sample) (((int32_t)(sample) >> 14) + MIC_CALIBRATION)
 
// Global variables ------------------------------------------------------------
 
static chirp_connect_t *chirp = NULL;
static chirp_connect_state_t currentState = CHIRP_CONNECT_STATE_NOT_CREATED;
static bool startTasks = false;
 
// Function definitions --------------------------------------------------------
 
void setupChirp();
void chirpErrorHandler(chirp_connect_error_code_t code);
void setupAudioInput(int sample_rate);
 
// Function declarations -------------------------------------------------------
 
void setup()
{
  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  delay(500);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
 
  Serial.begin(115200);
  Serial.printf("Heap size: %u\n", ESP.getFreeHeap());
 
  xTaskCreate(initTask, "initTask", 16384, NULL, 1, NULL);
}
 
void loop()
{
  esp_err_t audioError;
  chirp_connect_error_code_t chirpError;
 
  if (startTasks)
  {
    xTaskCreate(processInputTask, "processInputTask", 16384, NULL, 5, NULL);
    startTasks = false;
  }
}
 
// RTOS Tasks ------------------------------------------------------------------
 
void initTask(void *parameter)
{
  setupChirp();
 
  chirp_connect_error_code_t chirpError = chirp_connect_set_input_sample_rate(chirp, SAMPLE_RATE);
  chirpErrorHandler(chirpError);
  setupAudioInput(SAMPLE_RATE);
 
  Serial.printf("Heap size: %u\n", ESP.getFreeHeap());
  startTasks = true;
  vTaskDelete(NULL);
}
 
void processInputTask(void *parameter)
{
  esp_err_t audioError;
  chirp_connect_error_code_t chirpError;
 
  size_t bytesLength = 0;
  float buffer[BUFFER_SIZE] = {0};
  int32_t ibuffer[BUFFER_SIZE] = {0};
 
  while (currentState >= CHIRP_CONNECT_STATE_RUNNING)
  {
    audioError = i2s_read(I2S_NUM_0, ibuffer, BUFFER_SIZE * 4, &bytesLength, portMAX_DELAY);
    if (bytesLength)
    {
      for (int i = 0; i < bytesLength / 4; i++)
      {
        buffer[i] = (float) CONVERT_INPUT(ibuffer[i]);
      }
 
      chirpError = chirp_connect_process_input(chirp, buffer, bytesLength / 4);
      chirpErrorHandler(chirpError);
    }
  }
  vTaskDelete(NULL);
}
 
// Chirp -----------------------------------------------------------------------
 
void onStateChangedCallback(void *chirp, chirp_connect_state_t previous, chirp_connect_state_t current)
{
  currentState = current;
  Serial.printf("State changed from %d to %d\n", previous, current);
}
 
void onReceivingCallback(void *chirp, uint8_t *payload, size_t length, uint8_t channel)
{
  Serial.println("Receiving data...");
  digitalWrite(LED_PIN, HIGH);
}
 
void onReceivedCallback(void *chirp, uint8_t *payload, size_t length, uint8_t channel)
{
  if (payload)
  {
    char *data = (char *)calloc(length + 1, sizeof(uint8_t));
    memcpy(data, payload, length * sizeof(uint8_t));
    Serial.print("Received data: ");
    Serial.println(data);
    
    // text print
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    printEfont(data, 5, 10);
    
    free(data);
  }
  else
  {
    Serial.println("Decode failed.");
  }
}
 
void setupChirp()
{
  chirp = new_chirp_connect(CHIRP_APP_KEY, CHIRP_APP_SECRET);
  if (chirp == NULL)
  {
    Serial.println("Chirp initialisation failed.");
    return;
  }
 
  chirp_connect_error_code_t err = chirp_connect_set_config(chirp, CHIRP_APP_CONFIG);
  chirpErrorHandler(err);
 
  chirp_connect_callback_set_t callbacks = {0};
  callbacks.on_state_changed = onStateChangedCallback;
  callbacks.on_receiving = onReceivingCallback;
  callbacks.on_received = onReceivedCallback;
 
  err = chirp_connect_set_callbacks(chirp, callbacks);
  chirpErrorHandler(err);
 
  err = chirp_connect_set_callback_ptr(chirp, chirp);
  chirpErrorHandler(err);
 
  err = chirp_connect_start(chirp);
  chirpErrorHandler(err);
 
  Serial.println("Chirp Connect initialised.");
}
 
void chirpErrorHandler(chirp_connect_error_code_t code)
{
  if (code != CHIRP_CONNECT_OK)
  {
    const char *error_string = chirp_connect_error_code_to_string(code);
    Serial.println(error_string);
    exit(42);
  }
}
 
// I2S Audio -------------------------------------------------------------------
 
void setupAudioInput(int sample_rate)
{
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  sample_rate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
   };
 
   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_sample_rates(I2S_NUM_0, sample_rate);
   i2s_set_clk(I2S_NUM_0, sample_rate, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_MONO);
}