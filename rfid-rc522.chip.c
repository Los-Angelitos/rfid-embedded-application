#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t cs_pin; //permite saber cuando esp y chip pueden hablar. 
  uint32_t spi; //id del periferico SPI
  uint8_t spi_buffer[1]; //buffer de 1 byte, solo uno porque el chip 
  //responde caracter x caracter, se envia y recibe.
  uint8_t uid[4]; //los cuatro numeros del RFID
  int uid_pos; //la posicion al leer el RFID
} chip_state_t;

static void chip_pin_change(void *user_data, pin_t pin, uint32_t value);
static void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count);

//esto se ejecuta una sola vez, cunaod arranca el chip
void chip_init(void) {
  //se reserva memoria dinamica para guardar el estado del chip
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  //Inicializamos el cs pin, es lo que baja de low a high
  chip->cs_pin = pin_init("CS", INPUT_PULLUP);

  //observer del pin.
  const pin_watch_config_t watch_config = {
    //detectan subidas y bajadas del pin
    .edge = BOTH,
    .pin_change = chip_pin_change,
    //pasamos el puntero chip, que contiene todo lo del struct. 
    .user_data = chip,
  };
  pin_watch(chip->cs_pin, &watch_config);

//asignamos los pines fisicos a sus correspondientes. 
  const spi_config_t spi_config = {
    .sck = pin_init("SCK", INPUT),
    .miso = pin_init("MISO", INPUT),
    .mosi = pin_init("MOSI", INPUT),
    .done = chip_spi_done,
    .user_data = chip,
  };

  //se guarda en spi dentro de chip, el resultado de spi_init
  //que imprime un id. 
  chip->spi = spi_init(&spi_config);


  //aqui es como si se hiciera la lectura. 
  chip->uid[0] = 0xDE;
  chip->uid[1] = 0xAD;
  chip->uid[2] = 0xBE;
  chip->uid[3] = 0xEF;
  chip->uid_pos = -1;

  //y este es nuestro dummy
  chip->spi_buffer[0] = 0x00;


  printf("SPI Chip initialized!\n");
}

//el chip pin change se activa gracias al observer
void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  //recuperamos el chip desde el puntero user data que definimos antes
  chip_state_t *chip = (chip_state_t *)user_data;

  //verificamos que el pin que cambio sea efectivamente CS, es por protocolo 
  if (pin == chip->cs_pin) {
    //el esp32 quiere hablar. 
    if (value == LOW) {
      printf("SPI chip selected\n");
      chip->spi_buffer[0] = 0x00;
      //inicializamos el buffer y el chip escucha, esperando un byte del
      //esp32
      spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
    } else {
      //si el pin es high, la comunicacion termina. 
      printf("SPI chip deselected\n");
      chip->uid_pos=-1;
      spi_stop(chip->spi);
    }
  }
}

void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count) {
  //volvemos a agarrar el contenido en el puntero user data
  chip_state_t *chip = (chip_state_t *)user_data;
  //si no se recibio nada, termina
  if (!count) return;

  //declaramos un comando, que es el enviado por el microcontroller
  uint8_t cmd = buffer[0];

  //si el comando es tal, regresamos esto. 
  if (cmd == 0x26) {
    //pregunta ¿Hay tarjeta? aun no
    chip->spi_buffer[0] = 0x0A;
    chip->uid_pos = -1;
  } else if (cmd == 0x93) {
    // si el comando es de dame todo, le devuelve u byte
    chip->uid_pos = 0;
    chip->spi_buffer[0] = chip->uid[chip->uid_pos++];
  } else if (chip->uid_pos >= 0 && chip->uid_pos < 4) {
    // enviar siguiente byte del UID
    chip->spi_buffer[0] = chip->uid[chip->uid_pos++];
  } else {
    //si ya se acabaron los bytes del UID, toma 0
    chip->uid_pos = -1;
    chip->spi_buffer[0] = 0x00; // NAK
  }


  //si el pin sigue en low, el esp32 quiere enviar mas datos, asi que
  //reiniciamos la sesion spi con el nuevo contenido en el buffer.
  if (pin_read(chip->cs_pin) == LOW) {
    spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
  }
}
