// Ethercard LED example
#include <EtherCard.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,0,111 };

#define BUFFER_SIZE 500
byte Ethernet::buffer[BUFFER_SIZE];
BufferFiller bfill;

#define LED 2

bool ledStatus = false;


const char http_OK[] PROGMEM =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
    "HTTP/1.0 302 Found\r\n"
    "Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
    "HTTP/1.0 401 Unauthorized\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<h1>401 Unauthorized</h1>";

void homePage()
{
    bfill.emit_p(PSTR("$F"
        "$F"),
        http_OK,
        ledStatus?PSTR("1"):PSTR("0"),
        ledStatus?PSTR("on"):PSTR("off")
     );
 
}

void setup()
{
    Serial.begin(9600);
    
    pinMode(LED, OUTPUT);
    
    
    if (ether.begin(BUFFER_SIZE, mymac) == 0)
        Serial.println("Cannot initialise ethernet.");
    else
        Serial.println("Ethernet initialised.");

    ether.staticSetup(myip);
}

void loop()
{
    digitalWrite(LED, ledStatus);   // write to LED digital output
    
    delay(1);   // necessary for my system
    word len = ether.packetReceive();   // check for ethernet packet
    word pos = ether.packetLoop(len);   // check for tcp packet

    if (pos) {
        bfill = ether.tcpOffset();
        char *data = (char *) Ethernet::buffer + pos;
        
        if (strncmp("GET /", data, 5) != 0) {
            // Unsupported HTTP request
            // 304 or 501 response would be more appropriate
            bfill.emit_p(http_Unauthorized);
        }
        else {
            data += 5;
            
            if (data[0] == ' ') {
                // Return home page
                homePage();
            }
            else if (strncmp("?led=on ", data, 8) == 0) {
                // Set ledStatus true and redirect to home page
                ledStatus = true;
                bfill.emit_p(http_Found);
            }
            else if (strncmp("?led=off ", data, 9) == 0) {
                // Set ledStatus false and redirect to home page
                ledStatus = false;
                bfill.emit_p(http_Found);
            }
            else {
                // Page not found
                bfill.emit_p(http_Unauthorized);
            }
        }
        
        ether.httpServerReply(bfill.position());    // send http response
    }
}
