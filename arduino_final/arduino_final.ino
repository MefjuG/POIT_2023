#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;


bool sensorsActive;
const long interval = 1000;
unsigned long previousMillis = 0;
//Calibration data
int xCalAcc = -0.30;
int yCalAcc = 0.07;
int zCalAcc = 1;
int xCalGyr = 0.6;
int yCalGyr = 0.1;
int zCalGyr = 0;
int xCalAng = 0.6;
int yCalAng = 0.1;
//
float Total_angle_x, Total_angle_y;
float elapsedTime, time, timePrev;        //Variables for time control
//Gyro Variables
int gyro_error=0;                         //We use this variable to only calculate once the gyro data error
float Gyr_rawX, Gyr_rawY, Gyr_rawZ;     //Here we store the raw data read 
float Gyro_angle_x, Gyro_angle_y;         //Here we store the angle value obtained with Gyro data
float Gyro_raw_error_x, Gyro_raw_error_y; //Here we store the initial gyro data error

//Acc Variables
int acc_error=0; 
float Acc_rawX, Acc_rawY, Acc_rawZ;    //Here we store the raw data read 
float Acc_angle_x, Acc_angle_y;          //Here we store the angle value obtained with Acc data
float Acc_angle_error_x, Acc_angle_error_y; //Here we store the initial Acc data error
float rad_to_deg = 180/3.141592654;      //This value is for pasing from radians to degrees values

void setup(void) {
  sensorsActive = false;
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  //Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  
/*Here we calculate the acc data error before we start the loop
 * I make the mean of 200 values, that should be enough*/
  if(acc_error==0)
  {
    for(int a=0; a<200; a++)
    {
      Wire.beginTransmission(0x68);
      Wire.write(0x3B);                       //Ask for the 0x3B register- correspond to AcX
      Wire.endTransmission(false);
      Wire.requestFrom(0x68,6,true); 
      
      Acc_rawX=(Wire.read()<<8|Wire.read())/4096.0 ; //each value needs two registres
      Acc_rawY=(Wire.read()<<8|Wire.read())/4096.0 ;
      Acc_rawZ=(Wire.read()<<8|Wire.read())/4096.0 ;

      
      /*---X---*/
      Acc_angle_error_x = Acc_angle_error_x + ((atan((Acc_rawY)/sqrt(pow((Acc_rawX),2) + pow((Acc_rawZ),2)))*rad_to_deg));
      /*---Y---*/
      Acc_angle_error_y = Acc_angle_error_y + ((atan(-1*(Acc_rawX)/sqrt(pow((Acc_rawY),2) + pow((Acc_rawZ),2)))*rad_to_deg)); 
      
      if(a==199)
      {
        Acc_angle_error_x = Acc_angle_error_x/200;
        Acc_angle_error_y = Acc_angle_error_y/200;
        acc_error=1;
      }
    }
  }//end of acc error calculation   


/*Here we calculate the gyro data error before we start the loop
 * I make the mean of 200 values, that should be enough*/
  if(gyro_error==0)
  {
    for(int i=0; i<200; i++)
    {
      Wire.beginTransmission(0x68);            //begin, Send the slave adress (in this case 68) 
      Wire.write(0x43);                        //First adress of the Gyro data
      Wire.endTransmission(false);
      Wire.requestFrom(0x68,4,true);           //We ask for just 4 registers 
         
      Gyr_rawX=Wire.read()<<8|Wire.read();     //Once again we shif and sum
      Gyr_rawY=Wire.read()<<8|Wire.read();
   
      /*---X---*/
      Gyro_raw_error_x = Gyro_raw_error_x + (Gyr_rawX/32.8); 
      /*---Y---*/
      Gyro_raw_error_y = Gyro_raw_error_y + (Gyr_rawY/32.8);
      if(i==199)
      {
        Gyro_raw_error_x = Gyro_raw_error_x/200;
        Gyro_raw_error_y = Gyro_raw_error_y/200;
        gyro_error=1;
      }
    }
  }//end of gyro error calculation   

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  //Serial.print("Accelerometer range set to: ");
  /*switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }*/
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  //Serial.print("Gyro range set to: ");
  /*switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }*/

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  //Serial.print("Filter bandwidth set to: ");
  /*switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }*/

  Serial.println("");
  delay(100);
}

void sentData(float aX, float aY, float aZ,float gX, float gY, float gZ, float temp, float angX, float angY) {
  Serial.print("{\"aX\":");
  Serial.print(aX, 2);
  Serial.print(", \"aY\":");
  Serial.print(aY, 2);
  Serial.print(", \"aZ\":");
  Serial.print(aZ, 2);
  Serial.print(", \"gX\":");
  Serial.print(gX, 2);
  Serial.print(", \"gY\":");
  Serial.print(gY, 2);
  Serial.print(", \"gZ\":");
  Serial.print(gZ, 2);
  Serial.print(", \"temp\":");
  Serial.print(temp, 2);
  Serial.print(", \"angX\":");
  Serial.print(angX, 0);
  Serial.print(", \"AngY\":");
  Serial.print(angY, 0);
  Serial.println("}");
}

void loop() {

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (Serial.available()) {
    String command = Serial.readString();
    if (command.indexOf("Start")!= -1) {
      sensorsActive = true;
    } else if (command.indexOf("Stop")!= -1) {
      sensorsActive = false;
    } else if (command.startsWith("CALIBACC")) {

      // Parse the X, Y, and Z values from the input message
      int xIndex = command.indexOf("X=");
      int yIndex = command.indexOf("Y=");
      int zIndex = command.indexOf("Z=");

      String xValueString = command.substring(xIndex+2, yIndex-1);
      String yValueString = command.substring(yIndex+2, zIndex-1);
      String zValueString = command.substring(zIndex+2);

      // Convert the value strings to integers
      xCalAcc = xValueString.toInt();
      yCalAcc = yValueString.toInt();
      zCalAcc = zValueString.toInt();

    }else if (command.startsWith("CALIBGYRO")) {

      // Parse the X, Y, and Z values from the input message
      int xIndex = command.indexOf("X=");
      int yIndex = command.indexOf("Y=");
      int zIndex = command.indexOf("Z=");

      String xValueString = command.substring(xIndex+2, yIndex-1);
      String yValueString = command.substring(yIndex+2, zIndex-1);
      String zValueString = command.substring(zIndex+2);

      // Convert the value strings to integers
      xCalGyr = xValueString.toInt();
      yCalGyr = yValueString.toInt();
      zCalGyr = zValueString.toInt();

    }
    else if (command.startsWith("CALIBANG")) {

      // Parse the X, Y, and Z values from the input message
      int xIndex = command.indexOf("X=");
      int yIndex = command.indexOf("Y=");

      String xValueString = command.substring(xIndex+2, yIndex-1);
      String yValueString = command.substring(yIndex+2);

      // Convert the value strings to integers
      xCalAng = xValueString.toInt();
      yCalAng = yValueString.toInt();

    }
  }

  timePrev = time;                        // the previous time is stored before the actual time read
  time = millis();                        // actual time read
  elapsedTime = (time - timePrev) / 1000; //divide by 1000 in order to obtain seconds

  //////////////////////////////////////Gyro read/////////////////////////////////////

    Wire.beginTransmission(0x68);            //begin, Send the slave adress (in this case 68) 
    Wire.write(0x43);                        //First adress of the Gyro data
    Wire.endTransmission(false);
    Wire.requestFrom(0x68,4,true);           //We ask for just 4 registers
        
    Gyr_rawX=Wire.read()<<8|Wire.read();     //Once again we shif and sum
    Gyr_rawY=Wire.read()<<8|Wire.read();
    /*Now in order to obtain the gyro data in degrees/seconds we have to divide first
    the raw value by 32.8 because that's the value that the datasheet gives us for a 1000dps range*/
    /*---X---*/
    Gyr_rawX = (Gyr_rawX/32.8) - Gyro_raw_error_x; 
    /*---Y---*/
    Gyr_rawY = (Gyr_rawY/32.8) - Gyro_raw_error_y;
    
    /*---X---*/
    Gyro_angle_x = Gyr_rawX*elapsedTime;
    /*---X---*/
    Gyro_angle_y = Gyr_rawY*elapsedTime;

//////////////////////////////////////Acc read/////////////////////////////////////

  Wire.beginTransmission(0x68);     //begin, Send the slave adress (in this case 68) 
  Wire.write(0x3B);                 //Ask for the 0x3B register- correspond to AcX
  Wire.endTransmission(false);      //keep the transmission and next
  Wire.requestFrom(0x68,6,true);    //We ask for next 6 registers starting withj the 3B  

  Acc_rawX=(Wire.read()<<8|Wire.read())/4096.0 ; //each value needs two registres
  Acc_rawY=(Wire.read()<<8|Wire.read())/4096.0 ;
  Acc_rawZ=(Wire.read()<<8|Wire.read())/4096.0 ; 

 /*---X---*/
 Acc_angle_x = (atan((Acc_rawY)/sqrt(pow((Acc_rawX),2) + pow((Acc_rawZ),2)))*rad_to_deg) - Acc_angle_error_x;
 /*---Y---*/
 Acc_angle_y = (atan(-1*(Acc_rawX)/sqrt(pow((Acc_rawY),2) + pow((Acc_rawZ),2)))*rad_to_deg) - Acc_angle_error_y;    

 //////////////////////////////////////Total angle and filter/////////////////////////////////////
 /*---X axis angle---*/
 Total_angle_x = 0.98 *(Total_angle_x + Gyro_angle_x) + 0.02*Acc_angle_x;
 /*---Y axis angle---*/
 Total_angle_y = 0.98 *(Total_angle_y + Gyro_angle_y) + 0.02*Acc_angle_y;


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if(sensorsActive){
      sentData(a.acceleration.x+xCalAcc, a.acceleration.y+yCalAcc, a.acceleration.z+zCalAcc,g.gyro.x+xCalGyr,g.gyro.y+yCalGyr,g.gyro.z+zCalGyr,temp.temperature,Total_angle_x+xCalAng,Total_angle_y+yCalAng);
    }
  }

}