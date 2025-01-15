// set command numbers for communicating between ESP32 and Atmega16
#define NO 0x00
#define OK 0x01
#define READ_FS_TABLE 0xA1
#define READ_FILE 0xA2
#define WRITE_FILE 0xA3
#define FORMAT 0xA4

// macro for simple command transmitting
#define SEND_COMMAND(x) spi_transfer(x)

#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// Define the Slave Select pin
#define SS 5 

// setting the ESP32 as Master with default parameters
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);  // Define SPI settings: 1 MHz clock, MSB first, SPI mode 0


const char *ssid = "NAS_PROTO";
const char *password = "12345678";

WebServer server(80); // http web servre on port 80

// for file handling
String file_name;
String data;
int file_index = 0;

// run this function when receiving GET request on http://192.168.4.1:80/ (root)
void handleRoot() 
{
  // web page
  server.send(200, "text/html", "<!DOCTYPE html> <html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>File Management Website</title><style>body {font-family: 'Roboto', sans-serif;background-color: #f2f4f8;margin: 0;padding: 20px;}.container {max-width: 1000px;margin: auto;background: #ffffff;padding: 30px;border-radius: 10px;box-shadow: 0 5px 20px rgba(0, 0, 0, 0.1);display: grid;grid-template-columns: 1fr 1fr;gap: 20px;}.section {background: #f9fafb;padding: 20px;border-radius: 10px;text-align: center;}h1,h2 {text-align: center;color: #333;}input,textarea,button {width: 80%;margin: 15px auto;padding: 12px;border: 1px solid #ccc;border-radius: 5px;font-size: 16px;display: block;}input:focus,textarea:focus {border-color: #007bff;outline: none;box-shadow: 0 0 5px rgba(0, 123, 255, 0.5);}button {background-color: #212888;color: white;border: none;cursor: pointer;font-size: 16px;}button:hover {background-color: #287ba7;}pre {background: #f8f9fa;padding: 12px;border-radius: 5px;border: 1px solid #ccc;overflow-x: auto;}#fileList {list-style-type: none;padding: 0;}#fileList li {background-color: #e9ecef;margin: 10px 0;padding: 10px;border-radius: 5px;transition: background-color 0.3s;cursor: pointer;}#fileList li:hover {background-color: #287ba7;color: white;}textarea {resize: vertical;height: 200px;}/* Media query for responsiveness */@media (max-width: 768px) {.container {grid-template-columns: 1fr;}}</style> </head><body><div class=\"container\"><div class=\"section\"><h1>File Management</h1><form id=\"fileForm\"><input type=\"text\" id=\"fileName\" name=\"fileName\" placeholder=\"File Name\"><textarea id=\"fileContent\" name=\"fileContent\" placeholder=\"File Content\"></textarea><button type=\"button\" id=\"saveButton\">Save File</button></form></div><div class=\"section\"><h1>Existing Files</h1><ul id=\"fileList\"></ul><pre id=\"outputContent\"></pre></div></div><script>let files = {};let index = 0;document.getElementById('saveButton').addEventListener('click', () => {const fileName = document.getElementById('fileName').value;const fileContent = document.getElementById('fileContent').value;if (fileName && fileContent) {files[fileName] = index;index++;updateFileList();clearInputs();submitForm(fileName, fileContent);} else {alert('Please fill in all fields.');}});function updateFileList() {const fileList = document.getElementById('fileList');fileList.innerHTML = '';for (const fileName in files) {const listItem = document.createElement('li');listItem.textContent = fileName;listItem.style.cursor = 'pointer';listItem.addEventListener('click', () => {fetchFile(files[fileName]);});fileList.appendChild(listItem);}}function clearInputs() {document.getElementById('fileName').value = '';document.getElementById('fileContent').value = '';}function submitForm(fileName, fileContent) {const formData = new FormData();formData.append('fileName', fileName);formData.append('fileContent', fileContent);fetch('/', {method: 'POST',body: formData}).then(response => response.arrayBuffer()).then(buffer => {const decoder = new TextDecoder('ascii');const text = decoder.decode(buffer);return text;}).then(data => {console.log(data);});}function fetchFile(fileName) {fetch('/file?name=' + encodeURIComponent(fileName)).then(response => response.arrayBuffer()).then(buffer => {const decoder = new TextDecoder('ascii');const text = decoder.decode(buffer);return text;}).then(data => {document.getElementById('outputContent').textContent = data;});}// Fetch and display the file system tablefunction fetchFSTable() {fetch('/fs_table').then(response => response.json()).then(data => {const fileList = document.getElementById('fileList');fileList.innerHTML = '';data.forEach(entry => {const listItem = document.createElement('li');files[entry] = index;index++;listItem.textContent = entry;listItem.style.cursor = 'pointer';listItem.addEventListener('click', () => {fetchFile(entry);});fileList.appendChild(listItem);});});}// Call fetchFSTable on page load to update the file listfetchFSTable();</script> </body></html>");
}

// run this function when receving POST request on root
void handlePost()
{
    // check for file name and content existing in the http request.
    if (server.hasArg("fileName") && server.hasArg("fileContent"))
    {
        // get file name and content
        String fileName = server.arg("fileName");
        String fileContent = server.arg("fileContent");

        // send to Atmega16 Writing command
        uint8_t res = SEND_COMMAND(WRITE_FILE);

            // send the file name char by char
            for(uint8_t i = 0 ; i < 10 ; i ++)
            {
                // send 0xFF when the file name ends
                spi_transfer(i < fileName.length() ? fileName[i] : 0xFF);
            }

            // send the size of contents
            spi_transfer(fileContent.length());

            // send content char by char
            for(uint16_t i = 0 ; i < fileContent.length() ; i ++)
            {
                spi_transfer(fileContent[i]);
            }
            file_index++;
    }

    // send response to web page
    server.send(200, "text/plain", "POST request handled");
}

// run this function when receving GET request on getfile url http://192.168.4.1/file
void handleGetFile()
{
    // check for file name in the request
    if (server.hasArg("name"))
    {
        // get file name
        String req_index = server.arg("name");
        // send reading command
        spi_transfer(READ_FILE);
        // send index of the file
        spi_transfer(req_index[0]);
        // receive file size
        uint16_t size = spi_transfer(0x00);

        String content;

        // receive file content char by char
        for (uint16_t i = 0 ; i < size ; i++)
        {
          content += spi_transfer(0x00);
        }

        server.send(200, "text/plain", content);
    }
    else
    {
        server.send(400, "text/plain", "Bad request");
    }
}

void handleReadFSTable()
{
    // Send a command to the SPI slave to request the file system table
    uint8_t res = SEND_COMMAND(READ_FS_TABLE);
        String fsTable = "[";
        char receivedChar;
        bool firstEntry = true;
        String entry;

        do
        {
          entry = "";
            do
            {
                receivedChar = spi_transfer(0x00);  // Send a dummy byte to receive the next byte
                if (receivedChar != 0xFF)
                {
                    entry += receivedChar;
                }
            } while (receivedChar != 0xFF);  // End of the entry

            if (!entry.isEmpty())
            {
                if (!firstEntry)
                    fsTable += ",";

                fsTable += "[\"" + entry + "\"" + "," + String(file_index) + "]";
                file_index++;
                firstEntry = false;
            }

        } while (entry != "");  // Continue until an empty entry is received

        fsTable += "]";
        server.send(200, "application/json", fsTable);
    // else
    //     server.send(500, "text/plain", "Failed to retrieve file system table");
}

char spi_transfer(char data)
{
    digitalWrite(SS, LOW);                // Select the slave
    SPI.beginTransaction(settings);       // Begin the transaction
    char response = SPI.transfer(data);   // Send the data and receive a response
    SPI.endTransaction();                 // End the transaction
    digitalWrite(SS, HIGH);               // Deselect the slave
    return response;                      // Return the received response
}


void setup()
{
    Serial.begin(115200);

    SPI.begin();  // Initialize the SPI bus

    pinMode(SS, OUTPUT);  // Set the Slave Select pin as output
    digitalWrite(SS, HIGH);  // Deselect the slave

    Serial.println();
    Serial.println("Configuring access point...");

    if (!WiFi.softAP(ssid, password))
    {
        Serial.println("Soft AP creation failed.");
        while (1);
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handlePost);
    server.on("/file", HTTP_GET, handleGetFile);
    server.on("/fs_table", HTTP_GET, handleReadFSTable);
    server.begin();

    Serial.println("Server started");

}

bool f = false;

void loop()
{
    server.handleClient();

    // if(server.client())
    // {
    //   if(!f)
    //   {
    //     handleReadFSTable();
    //     f = true;
    //   }
    // }
    // else
    // {
    //   f = false;
    // }
    // byte dataToSend = 0x42;  // Data to send (example value)
    // byte receivedData = spi_transfer(dataToSend);  // Send and receive data

    // Serial.print("Sent: 0x");
    // Serial.print(dataToSend, HEX);
    // Serial.print(", Received: 0x");
    // Serial.println(receivedData, HEX);

    // delay(10000);  // Wait a second before the next transaction
}
