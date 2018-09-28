/********************************************************************************************\
  Init critical variables for logging (important during initial factory reset stuff )
  \*********************************************************************************************/
void initLog()
{
  //make sure addLog doesnt do any stuff before initalisation of Settings is complete.
  Settings.UseSerial=true;
  Settings.SyslogFacility=0;
  setLogLevelFor(LOG_TO_SYSLOG, 0);
  setLogLevelFor(LOG_TO_SERIAL, 2); //logging during initialisation
  setLogLevelFor(LOG_TO_WEBLOG, 2);
  setLogLevelFor(LOG_TO_SDCARD, 0);
}

/********************************************************************************************\
  Logging
  \*********************************************************************************************/
String getLogLevelDisplayString(int logLevel) {
  switch (logLevel) {
    case LOG_LEVEL_NONE:       return F("None");
    case LOG_LEVEL_ERROR:      return F("Error");
    case LOG_LEVEL_INFO:       return F("Info");
    case LOG_LEVEL_DEBUG:      return F("Debug");
    case LOG_LEVEL_DEBUG_MORE: return F("Debug More");
    case LOG_LEVEL_DEBUG_DEV:  return F("Debug dev");

    default:
      return "";
  }
}

String getLogLevelDisplayStringFromIndex(byte index, int& logLevel) {
  switch (index) {
    case 0: logLevel = LOG_LEVEL_ERROR;      break;
    case 1: logLevel = LOG_LEVEL_INFO;       break;
    case 2: logLevel = LOG_LEVEL_DEBUG;      break;
    case 3: logLevel = LOG_LEVEL_DEBUG_MORE; break;
    case 4: logLevel = LOG_LEVEL_DEBUG_DEV;  break;

    default: logLevel = -1; return "";
  }
  return getLogLevelDisplayString(logLevel);
}

void addToLog(byte loglevel, const String& string)
{
  addToLog(loglevel, string.c_str());
}

void addToLog(byte logLevel, const __FlashStringHelper* flashString)
{
    checkRAM(F("addToLog"));
    String s(flashString);
    addToLog(logLevel, s.c_str());
}

void disableSerialLog() {
  log_to_serial_disabled = true;
  setLogLevelFor(LOG_TO_SERIAL, 0);
}

void setLogLevelFor(byte destination, byte logLevel) {
  switch (destination) {
    case LOG_TO_SERIAL:
      if (!log_to_serial_disabled || logLevel == 0)
        Settings.SerialLogLevel = logLevel; break;
    case LOG_TO_SYSLOG: Settings.SyslogLevel = logLevel;    break;
    case LOG_TO_WEBLOG: Settings.WebLogLevel = logLevel;    break;
    case LOG_TO_SDCARD: Settings.SDLogLevel = logLevel;     break;
    default:
      break;
  }
  updateLogLevelCache();
}

void updateLogLevelCache() {
  byte max_lvl = 0;
  if (!log_to_serial_disabled && serialLogActiveRead()) {
    max_lvl = _max(max_lvl, Settings.SerialLogLevel);
    if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
      Serial.setDebugOutput(true);
  } else {
    Serial.setDebugOutput(false);
  }
  max_lvl = _max(max_lvl, Settings.SyslogLevel);
  if (Logging.logActiveRead()) {
    max_lvl = _max(max_lvl, Settings.WebLogLevel);
  }
#ifdef FEATURE_SD
  max_lvl = _max(max_lvl, Settings.SDLogLevel);
#endif
  highest_active_log_level = max_lvl;
}

bool loglevelActiveFor(byte logLevel) {
  return loglevelActive(logLevel, highest_active_log_level);
}

byte getSerialLogLevel() {
  byte logLevelSettings = 0;
  if (!Settings.UseSerial) return 0;
  if (log_to_serial_disabled || log_to_serial_disabled_temporary) return 0;
  logLevelSettings = Settings.SerialLogLevel;
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED){
    logLevelSettings = 2;
  }
/*
  if (!serialLogActiveRead()) {
    if (logLevelSettings != 0) {
      updateLogLevelCache();
    }
    logLevelSettings = 0;
  }
*/
  return logLevelSettings;
}

byte getWebLogLevel() {
  byte logLevelSettings = 0;
  if (Logging.logActiveRead()) {
    logLevelSettings = Settings.WebLogLevel;
  } else {
    if (Settings.WebLogLevel != 0) {
      updateLogLevelCache();
    }
  }
  return logLevelSettings;
}

boolean loglevelActiveFor(byte destination, byte logLevel) {
  byte logLevelSettings = 0;
  switch (destination) {
    case LOG_TO_SERIAL: {
      logLevelSettings = getSerialLogLevel();
      break;
    }
    case LOG_TO_SYSLOG: {
      logLevelSettings = Settings.SyslogLevel;
      break;
    }
    case LOG_TO_WEBLOG: {
      logLevelSettings = getWebLogLevel();
      break;
    }
    case LOG_TO_SDCARD: {
      #ifdef FEATURE_SD
      logLevelSettings = Settings.SDLogLevel;
      #endif
      break;
    }
    default:
      return false;
  }
  return loglevelActive(logLevel, logLevelSettings);
}


boolean loglevelActive(byte logLevel, byte logLevelSettings) {
  return (logLevel <= logLevelSettings);
}

void addToLog(byte logLevel, const char *line)
{
  const size_t line_length = strlen(line);
  if (loglevelActiveFor(LOG_TO_SERIAL, logLevel)) {
    int roomLeft = ESP.getFreeHeap() - 5000;
    if (roomLeft > 0) {
      String timestamp_log(millis());
      timestamp_log += F(" : ");
      for (size_t i = 0; i < timestamp_log.length(); ++i) {
        serialLogBuffer.push_back(timestamp_log[i]);
      }
      size_t pos = 0;
      while (pos < line_length && pos < static_cast<size_t>(roomLeft)) {
        serialLogBuffer.push_back(line[pos]);
        ++pos;
      }
      serialLogBuffer.push_back('\r');
      serialLogBuffer.push_back('\n');
    }
  }
  if (loglevelActiveFor(LOG_TO_SYSLOG, logLevel)) {
    syslog(logLevel, line);
  }
  if (loglevelActiveFor(LOG_TO_WEBLOG, logLevel)) {
    Logging.add(logLevel, line);
  }

#ifdef FEATURE_SD
  if (loglevelActiveFor(LOG_TO_SDCARD, logLevel)) {
    File logFile = SD.open("log.dat", FILE_WRITE);
    if (logFile)
      logFile.println(line);
    logFile.close();
  }
#endif
}

void process_serialLogBuffer() {
  if (serialLogBuffer.size() == 0) return;
  size_t snip = 128; // Some default, ESP32 doesn't have the availableForWrite function yet.
#if defined(ESP8266)
  snip = Serial.availableForWrite();
#endif
  if (snip > 0) {
    last_serial_log_read = millis();
    size_t bytes_to_write = serialLogBuffer.size();
    if (snip < bytes_to_write) bytes_to_write = snip;
    for (size_t i = 0; i < bytes_to_write; ++i) {
      Serial.write(serialLogBuffer.front());
      serialLogBuffer.pop_front();
    }
  }
}

void tempDisableSerialLog(bool setToDisabled) {
  if (log_to_serial_disabled_temporary == setToDisabled) return;

  // FIXME TD-er: For some reason disabling serial log will not enable it again.
  //  log_to_serial_disabled_temporary = setToDisabled;
  //  updateLogLevelCache();
}

bool serialLogActiveRead() {
  if (!Settings.UseSerial) return false;
  // Some default, ESP32 doesn't have the availableForWrite function yet.
  // Not sure how to detect read activity on an ESP32.
  size_t tx_free = 128;
#if defined(ESP8266)
  tx_free = Serial.availableForWrite();
#endif
  static size_t prev_tx_free = 0;
  if (tx_free < prev_tx_free) {
    prev_tx_free = tx_free;
    tempDisableSerialLog(false);
    return true;
  }
  // Must always set it or else it will never recover from prev_tx_free == 0
  prev_tx_free = tx_free;
  if (timePassedSince(last_serial_log_read) > LOG_BUFFER_EXPIRE) {
    serialLogBuffer.clear();
    // Just add some marker to get it going again when the serial buffer is
    // read again and the serial log level was temporary set to 0 since nothing was read.
    if (Settings.SerialLogLevel > 0) {
      serialLogBuffer.push_back('\n');
    }
    tempDisableSerialLog(true);
    return false;
  }
  return true;
}

