#include<iostream>
#include<fstream>
#include<string>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<wiringPiI2C.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#define BUFFER_SIZE 64      
using namespace std;
// the time is in the registers in encoded decimal form
int bcdToDec(char b) { return (b/16)*10 + (b%16); }

int main(){
   int file;
   printf("Starting the DS3231 test application\n");
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){
      perror("failed to open the bus\n");
      return 1;
   }
   if(ioctl(file, I2C_SLAVE, 0x53) < 0){
      perror("Failed to connect to the sensor\n");
      return 1;
   }
   char writeBuffer[1] = {0x00};
   if(write(file, writeBuffer, 1)!=1){
      perror("Failed to reset the read address\n");
      return 1;
   }
   char buf[BUFFER_SIZE];
   if(read(file, buf, BUFFER_SIZE)!=BUFFER_SIZE){
      perror("Failed to read in the buffer\n");
      return 1;
   }
   int datax1 = bcdToDec(buf[51]);
   int datax2 = bcdToDec(buf[52]);
   int datay1 = bcdToDec(buf[53]);
   int datay2 = bcdToDec(buf[54]);
   int dataz1 = bcdToDec(buf[55]);
   int dataz2 = bcdToDec(buf[56]);



   cout << "The X-Axis is " << datax1 << " " << datax2 << endl; 
   cout << "The Y-Axis is " << datay1 << " " << datay2 << endl;
   cout << "The Z-Axis is " << dataz1 << " " << dataz2 << endl;   
   QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
   QJsonObject obj = doc.object();
   QJsonObject sample = obj["sample"].toObject();
   this->datax1 = (float) sample["datax1"].toDouble();
   this->datax2 = (float) sample["datax2"].toDouble();
   cout << "The datax1 " << datax1 << " and datax2 is "
    	<< datax2 << endl;
  
   close(file);
   return 0;
}
