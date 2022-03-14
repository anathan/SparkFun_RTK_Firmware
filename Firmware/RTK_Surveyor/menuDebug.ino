//Toggle control of heap reports and I2C GNSS debug
void menuDebug()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Debug Menu"));

    printModuleInfo();

    printCurrentConditions();

    Serial.print(F("1) I2C Debugging Output: "));
    if (settings.enableI2Cdebug == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Heap Reporting: "));
    if (settings.enableHeapReport == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("3) Task Highwater Reporting: "));
    if (settings.enableTaskReports == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("4) Set SPI/SD Interface Frequency: "));
    Serial.print(settings.spiFrequency);
    Serial.println(" MHz");

    Serial.print(F("5) Set SPP RX Buffer Size: "));
    Serial.println(settings.sppRxQueueSize);

    Serial.print(F("6) Set SPP TX Buffer Size: "));
    Serial.println(settings.sppTxQueueSize);

    Serial.print(F("7) Throttle BT Transmissions During SPP Congestion: "));
    if (settings.throttleDuringSPPCongestion == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("8) Display Reset Counter: "));
    if (settings.enableResetDisplay == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("r) Force system reset"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.enableI2Cdebug ^= 1;

      if (settings.enableI2Cdebug)
        i2cGNSS.enableDebugging(Serial, true); //Enable only the critical debug messages over Serial
      else
        i2cGNSS.disableDebugging();
    }
    else if (incoming == '2')
    {
      settings.enableHeapReport ^= 1;
    }
    else if (incoming == '3')
    {
      settings.enableTaskReports ^= 1;
    }
    else if (incoming == '4')
    {
      Serial.print(F("Enter SPI frequency in MHz (1 to 48): "));
      int freq = getNumber(menuTimeout); //Timeout after x seconds
      if (freq < 1 || freq > 48) //Arbitrary 48 hour limit
      {
        Serial.println(F("Error: SPI frequency out of range"));
      }
      else
      {
        settings.spiFrequency = freq; //Recorded to NVM and file at main menu exit
      }
    }
    else if (incoming == '5')
    {
      Serial.print(F("Enter SPP RX Queue Size in Bytes (32 to 16384): "));
      uint16_t queSize = getNumber(menuTimeout); //Timeout after x seconds
      if (queSize < 32 || queSize > 16384) //Arbitrary 16k limit
      {
        Serial.println(F("Error: Queue size out of range"));
      }
      else
      {
        settings.sppRxQueueSize = queSize; //Recorded to NVM and file at main menu exit
      }
    }
    else if (incoming == '6')
    {
      Serial.print(F("Enter SPP TX Queue Size in Bytes (32 to 16384): "));
      uint16_t queSize = getNumber(menuTimeout); //Timeout after x seconds
      if (queSize < 32 || queSize > 16384) //Arbitrary 16k limit
      {
        Serial.println(F("Error: Queue size out of range"));
      }
      else
      {
        settings.sppTxQueueSize = queSize; //Recorded to NVM and file at main menu exit
      }
    }
    else if (incoming == '7')
    {
      settings.throttleDuringSPPCongestion ^= 1;
    }
    else if (incoming == '8')
    {
      settings.enableResetDisplay ^= 1;
      if (settings.enableResetDisplay == true)
      {
        settings.resetCount = 0;
        recordSystemSettings(); //Record to NVM
      }
    }
    else if (incoming == 'r')
    {
      ESP.restart();
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
    {
      break;
    }
    else
      printUnknown(incoming);
  }

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars
}

//Print the current long/lat/alt/HPA/SIV
//From Example11_GetHighPrecisionPositionUsingDouble
void printCurrentConditions()
{
  // getHighResLatitude: returns the latitude from HPPOSLLH as an int32_t in degrees * 10^-7
  // getHighResLatitudeHp: returns the high resolution component of latitude from HPPOSLLH as an int8_t in degrees * 10^-9
  // getHighResLongitude: returns the longitude from HPPOSLLH as an int32_t in degrees * 10^-7
  // getHighResLongitudeHp: returns the high resolution component of longitude from HPPOSLLH as an int8_t in degrees * 10^-9
  // getElipsoid: returns the height above ellipsoid as an int32_t in mm
  // getElipsoidHp: returns the high resolution component of the height above ellipsoid as an int8_t in mm * 10^-1
  // getMeanSeaLevel: returns the height above mean sea level as an int32_t in mm
  // getMeanSeaLevelHp: returns the high resolution component of the height above mean sea level as an int8_t in mm * 10^-1
  // getHorizontalAccuracy: returns the horizontal accuracy estimate from HPPOSLLH as an uint32_t in mm * 10^-1

  // First, let's collect the position data
  int32_t latitude = i2cGNSS.getHighResLatitude();
  int8_t latitudeHp = i2cGNSS.getHighResLatitudeHp();
  int32_t longitude = i2cGNSS.getHighResLongitude();
  int8_t longitudeHp = i2cGNSS.getHighResLongitudeHp();
  int32_t ellipsoid = i2cGNSS.getElipsoid();
  int8_t ellipsoidHp = i2cGNSS.getElipsoidHp();
  int32_t msl = i2cGNSS.getMeanSeaLevel();
  int8_t mslHp = i2cGNSS.getMeanSeaLevelHp();
  uint32_t accuracy = i2cGNSS.getHorizontalAccuracy();
  byte SIV = i2cGNSS.getSIV();

  // Defines storage for the lat and lon as double
  double d_lat; // latitude
  double d_lon; // longitude

  // Assemble the high precision latitude and longitude
  d_lat = ((double)latitude) / 10000000.0; // Convert latitude from degrees * 10^-7 to degrees
  d_lat += ((double)latitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )
  d_lon = ((double)longitude) / 10000000.0; // Convert longitude from degrees * 10^-7 to degrees
  d_lon += ((double)longitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )

  // Print the lat and lon
  Serial.print("Lat (deg): ");
  Serial.print(d_lat, 9);
  Serial.print(", Lon (deg): ");
  Serial.print(d_lon, 9);

  // Now define float storage for the heights and accuracy
  float f_ellipsoid;
  float f_msl;
  float f_accuracy;

  // Calculate the height above ellipsoid in mm * 10^-1
  f_ellipsoid = (ellipsoid * 10) + ellipsoidHp;
  // Now convert to m
  f_ellipsoid = f_ellipsoid / 10000.0; // Convert from mm * 10^-1 to m

  // Calculate the height above mean sea level in mm * 10^-1
  f_msl = (msl * 10) + mslHp;
  // Now convert to m
  f_msl = f_msl / 10000.0; // Convert from mm * 10^-1 to m

  // Convert the horizontal accuracy (mm * 10^-1) to a float
  f_accuracy = accuracy;
  // Now convert to m
  f_accuracy = f_accuracy / 10000.0; // Convert from mm * 10^-1 to m

  // Finally, do the printing
  Serial.print(", Ellipsoid (m): ");
  Serial.print(f_ellipsoid, 4); // Print the ellipsoid with 4 decimal places

  Serial.print(", Mean Sea Level (m): ");
  Serial.print(f_msl, 4); // Print the mean sea level with 4 decimal places

  Serial.print(", Accuracy (m): ");
  Serial.print(f_accuracy, 4); // Print the accuracy with 4 decimal places

  Serial.print(F(", SIV: "));
  Serial.print(SIV);

  Serial.println();
}
