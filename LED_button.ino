/* Note: the page has to be reloaded manually if you want to see changed LED or button state */

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 178); // depends on your ip adress
EthernetServer server(80);  // creating the server on port 80

const int buttonPin = 8;
const int ledPin = 2;

int ledState = LOW;         // current state of LED pin
int buttonState;             // current state of button pin
int lastButtonState = HIGH;   // last reading from button pin


unsigned long lastDebounceTime = 0;  // last time when the button was pressed
unsigned long debounceDelay = 50;    // debounce time

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, ledState);
  Ethernet.begin(mac, ip); // connecting to the network
}

void loop() {
  EthernetClient client = server.available();
  
  // reading current button state into a variable
  // returns low when the button is pressed, high when not pressed
  int reading = digitalRead(buttonPin);

  // checking if the button was pressed

  // if the state changed when the button was pressed or debounce effect
  if (reading != lastButtonState) {
    // reset debounce timer
    lastDebounceTime = millis();
  }


  if ((millis() - lastDebounceTime) > debounceDelay) {
    // the state that was read from the button pin stayed long enough (longer from debounceDelay time)
    // that's why it's considered as the right input

    // if the button state changed
    if (reading != buttonState) {
      buttonState = reading;

      // change LED state only if the button was pressed
      if (buttonState == LOW) {
        ledState = !ledState;
      }
    }
  }

  // set LED state
  digitalWrite(ledPin, ledState);

  // saving the button state for next iteration trough the loop
  lastButtonState = reading;

  while (client.connected()) {
    if (client.available()) {
      String request = client.readStringUntil('\r');
      Serial.println(request);
      client.flush();
      Serial.begin(9600);


      // checking the type of the request, should the LED be on or off
      if (request.indexOf("/LED=ON") != -1)  {
        digitalWrite(ledPin, HIGH);
        ledState = HIGH;
      }
      else if (request.indexOf("/LED=OFF") != -1)  {
        digitalWrite(ledPin, LOW);
        ledState = LOW;
      }

      // web server answer
      client.println("");
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println("<head>");
      client.println("<title> Arduino Web Server </title>");
      client.println("</head>");

      client.println("<style>");
      client.println("button{ font-size: 15px; }");
      client.println(".light{color: black; background-color: green;}" );
      client.println(".dark{color: white; background-color: black;}" );
      client.println("h1,h2{font-size: 60px;}" );
      client.println("</style>");

      client.println("<body>");
      client.println("<h1>LED control</h1>");
      client.println("<h2>LED is currently: ");

      if ( ledState == HIGH ) {
        client.println("ON </h2>");
        client.println("<form action= \"/LED=OFF\">");
        client.println("<button> <h2> Turn off </h2> </button>");
        client.println("</form>");
      }
      else if ( ledState == LOW ) {
        client.println("OFF </h2>");
        client.println("<form action= \"/LED=ON\">");
        client.println("<button> <h2> Turn on </h2> </button></a>");
        client.println("</form>");
      }

      client.println("<br>");
      client.println("<hr style='border: 1px solid red;'>");

      client.println("<h2>Button is: ");
      if (buttonState == LOW) {
        client.println("<h2 class='light'> ON </h1>");
      }
      else {
        client.println("<h2 class='dark'> OFF </h1>");
      }

      client.println("</body>");
      client.println("</html>");

      // closing the connection
      client.stop();
    }
  }
}
