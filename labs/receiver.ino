/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000002ELL, 0x000000002FLL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

int walls=0;
void setup(void)
{
//  printf("reset");
//  delay(1000);
// 
 

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  printf_begin();
//  printf("\n\rRF24/examples/GettingStarted/\n\r");
//  printf("ROLE: %s\n\r",role_friendly_name[role]);
//  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
//
//  
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MAX);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);


  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  //radio.printDetails();
}

void loop(void)
{

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
 
    //Serial.println("reset");
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      word got_update;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &got_update, sizeof(word) );

        //Bit masks for received data
        word explored=got_update>>15;
        word walls=got_update>>11&15; //  1111
        word treasure=got_update>>8&8; //  111;
        word robot=got_update>>7&1;
        word loc=got_update&127; //127 is 1111111
        word x=loc%9;
        int y=loc/9-0.5;
        char c=',';

        //Print in GUI format
        Serial.print(x);
        Serial.print(c);
        Serial.print(y);
        Serial.print(c);
        if ((walls&8)>>3){Serial.print("north=true,");}
        else {Serial.print("north=false,");}
        if ((walls&3)>>2){Serial.print("south=true,");}
        else {Serial.print("south=false,");}
        if ((walls&2)>>1){Serial.print("west=true,");}
        else {Serial.print("west=false,");}
        if (walls&1){Serial.print("east=true");}
        else {Serial.print("east=false");}
        Serial.print("\n");

      }
      delay(20);
      
      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      int confirm=6;
      if (radio.write( &confirm, sizeof(confirm) )){
         }

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp