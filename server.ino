/* Web_HelloWorld.pde - very simple Webduino example */

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"
#include <stdio.h>
#include <stdlib.h>

#include <IRremote.h>

IRsend irsend;
int recvPin = 2;
IRrecv irrecv(recvPin);

/* CHANGE THIS TO YOUR OWN UNIQUE VALUE.  The MAC number should be
 * different from any other devices on your network or you'll have
 * problems receiving packets. */
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

void newCode(WebServer &server, unsigned long& code, int& irType);
/* CHANGE THIS TO MATCH YOUR HOST NETWORK.  Most home networks are in
 * the 192.168.0.XXX or 192.168.1.XXX subrange.  Pick an address
 * that's not in use and isn't going to be automatically allocated by
 * DHCP from your router. */
static uint8_t ip[] = { 10,100,102,5 };

/* This creates an instance of the webserver.  By specifying a prefix
 * of "", all pages will be at the root of the server. */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

/* commands are functions that get called by the webserver framework
 * they can read any posted data from client, and they output to the
 * server to send data back to the web browser. */
void helloCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[32];
  char value[32];
  bool enter = true;
  unsigned long code = 0;
  int irType = 0;
  String msg = "";
  server.httpSuccess();
  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */

  if (strlen(url_tail))
    {
    while (strlen(url_tail))
      {
      rc = server.nextURLparam(&url_tail, name, 32, value,32);

      if (rc == URLPARAM_EOS)
        server.printP("end");
       else
       {
        if(strcmp(name, "data") == 0) {
          code = strtoul (value, NULL, 0);
          }
         else if(strcmp(name, "type") == 0) {
            irType = (int)(value[0] - '0');
          }
         else if(strcmp(name, "listen") == 0) {
          enter = false;
          newCode(server, code, irType);
         }
        }
      }
    }

  if (code && irType && enter) {
    switch (irType) {
      case 3:
      irsend.sendNEC(code,0);
      msg = "NEC sent";
      break;
      case 7:
      irsend.sendSAMSUNG(code,32);
      msg = "Samsung sent";
      break;
      default:
      msg = "Invalid Type";
    }
  }
  else if (!enter) {
    msg = "{ \"code\" : " + String(code) + String(", \"type\" : ") + String(irType) + " }";
  }
  else{
    msg = "nothing happend";
  }
  if (type != WebServer::HEAD)
  {
    server.print(msg);
  }
}
void newCode(WebServer &server, unsigned long& code, int& irType) {
  decode_results  results;   // Somewhere to store the results
  bool goodCode = false;
  unsigned long start = millis();
  unsigned long passed = millis();
  while(passed - start < 5000 && !goodCode ) {
      if (irrecv.decode(&results)) {  // Grab an IR code
        if(results.decode_type != UNKNOWN) {
          irType = results.decode_type;
          code = results.value;
          goodCode = true;
         }
      }
      passed = millis();
  }
  irrecv.resume();
}
void setup()
{
  /* initialize the Ethernet adapter */
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  irrecv.enableIRIn();
  /* setup our default command that will be run when the user accesses
   * the root page on the server */
  webserver.setDefaultCommand(&helloCmd);
  /* run the same command if you try to load /index.html, a common
   * default page name */
  webserver.addCommand("index.html", &helloCmd);

  /* start the webserver */
  webserver.begin();
}

void loop()
{
  char buff[64];
  int len = 64;

  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);
}
