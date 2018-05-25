/* This file is a template for Plugins */

/* References:
 https://www.letscontrolit.com/wiki/index.php/ESPEasyDevelopment
 https://www.letscontrolit.com/wiki/index.php/ESPEasyDevelopmentGuidelines
 https://github.com/letscontrolit/ESPEasyPluginPlayground
 https://diyprojects.io/esp-easy-develop-plugins/

 A Plugin should have an ID.
 The official plugin list is available here: https://www.letscontrolit.com/wiki/index.php/Official_plugin_list
 The plugin playground is available here: https://github.com/letscontrolit/ESPEasyPluginPlayground

 Use the next available ID. The maximum number of Plugins is defined in ESPEasy-Globals.h (PLUGIN_MAX)

 The Plugin filename should be of the form "_Pxxx_name.ino", where:
    xxx is the ID
    <name> is a short name of the Plugin
 As an example: "_P001_Switch.ino"

Hints for plugin development:
 - plugins should ideally be added without changes in the framework
 - avoid including libraries. Include the necessary code in the plugin
 - when verifying the plugin check the following:
     - memory used (try different scenarios: plugin enabled, plugin in use, commands executed, plugin disabled, device added/removed)
     - other tests??
 - the development life-cycle is:
     - implement plugin and perform testing
     - set plugin status to DEVELOPMENT and distribute to other users for testing
     - after sufficient usage and possible code correction, set plugin status to TESTING and perform testing with more users
     - finally, plugin will be accepted in project
 - along with the plugin source code, prepare a wiki page containing:
     - instructions on how to make the necessary configuration
     - instructions on commands (if any)
     - examples: plugin usage, command usage,...
 - when a plugin is removed (deleted), make sure you free any memory it uses. Use PLUGIN_EXIT for that
 - if your plugin creates log entries, prefix your entries with your plugin id: "[Pxxx] my plugin did this"
 - if your plugin takes input from user and/or accepts/sends http commands, make sure you properly handle non-alphanumeric characters correctly
 - After ESP boots, all devices can send data instantly. If your plugin is for a sensor which sends data, ensure it doesn't need a delay before receiving data
 - ensure the plugin does not create sideffects (eg. crashes) if there's no actual device connected
- check the device's return values. Ensure that if the device returns an invalid value, to use a value like 0 or null to avoid side-effects
- extra hints mentioned in: https://github.com/letscontrolit/ESPEasy/issues/698
*/
#define USES_P078

#include <Arduino.h>
//#include <ESPEasy-Globals.h>
#ifdef USES_P078
//#include section
//include libraries here. For example:
#include <ctype.h>
#include <time.h>

extern "C"
{
#include "ccronexpr.h"
}
//#include <stdio.h>
//#include <string.h>

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_078
#define PLUGIN_ID_078      78                 //plugin id
#define PLUGIN_NAME_078   "CRON"              //"Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_078 ""              //variable output of the plugin. The label is in quotation marks
#define PLUGIN_078_DEBUG  true                //set to true for extra log info in the debug


typedef struct 
{
  char Expression[41];
  time_t LastExecution;
  time_t NextExecution;
} CronState;

//A plugin has to implement the following function

boolean Plugin_078(byte function, struct EventStruct *event, String& string)
{
  //function: reason the plugin was called
  //event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
        //This case defines the device characteristics, edit appropriately

        Device[++deviceCount].Number = PLUGIN_ID_078;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;  //how the device is connected
        Device[deviceCount].VType = SENSOR_TYPE_NONE; //type of value the plugin will return, used only for Domoticz
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;             //number of output variables. The value should match the number of keys PLUGIN_VALUENAME1_xxx
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      //return the device name
      string = F(PLUGIN_NAME_078);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      //called when the user opens the module configuration page
      //it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_078));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      CronState state;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      
      addFormTextBox(F("CRON Expression")
        , F("plugin_078_cron_exp")
        , state.Expression
        , 39);

      addFormNote(String(F("Last execution:")) + getDateTimeString(*gmtime(&state.LastExecution)));
      addFormNote(String(F("Next execution:")) + getDateTimeString(*gmtime(&state.NextExecution)));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      CronState state;
      String log;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      char expression[40];
      strncpy(expression,  WebServer.arg(F("plugin_078_cron_exp")).c_str() , sizeof(expression));
      if(strcmp(expression, state.Expression) != 0)
      {
        strncpy(state.Expression, expression, sizeof(state.Expression));
        cron_expr expr;
        const char* err = NULL;
        //memset(&expr, 0, sizeof(expr));
        cron_parse_expr(expression, &expr, &err);
        if (!err) 
        {
          state.LastExecution = mktime((struct tm *)&tm); // Set current time;
          state.NextExecution = cron_next((cron_expr *)&expr, state.LastExecution);          
        }
        else
        {          
          log = String(F("CRON Expression: Error ")) + String(err);
          addHtmlError(log);
          addLog(LOG_LEVEL_ERROR, log);
        }
        log = SaveCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
        if(log != F("")) 
        {
            log = String(PSTR(PLUGIN_NAME_078)) + String(F(": Saving ")) + log;
            addLog(LOG_LEVEL_ERROR, log);
        }
      }
      success = true;
      break;

    }
    case PLUGIN_INIT:
    {
      //this case defines code to be executed when the plugin is initialised

      //after the plugin has been initialised successfuly, set success and break
      success = true;
      break;

    }

    case PLUGIN_READ:
    {
      //code to be executed to read data
      //It is executed according to the delay configured on the device configuration page, only once

      //after the plugin has read data successfuly, set success and break
      success = true;
      break;

    }

    case PLUGIN_WRITE:
    {
       break;
    }

    case PLUGIN_EXIT:
    {
      //perform cleanup tasks here. For example, free memory

      break;

    }

    case PLUGIN_ONCE_A_SECOND:
    {
      //code to be executed once a second. Tasks which do not require fast response can be added here
      CronState state;
      String log;
      LoadCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
      struct tm current = tm;
      #ifdef PLUGIN_078_DEBUG
        Serial.println(F("CRON Debug info:"));
        Serial.print(F("Next execution:"));
        Serial.println(getDateTimeString(*gmtime(&state.NextExecution)));
        Serial.print(F("Current Time:"));
        Serial.println(getDateTimeString(current));
        Serial.print(F("Triggered:"));
        Serial.println(state.NextExecution <  mktime((struct tm *)&current));      
      #endif
      if(state.NextExecution <=  mktime((struct tm *)&current))
      {
        cron_expr expr;
        const char* err = NULL;
        //memset(&expr, 0, sizeof(expr));
        addLog(LOG_LEVEL_ERROR, F("Cron Elapsed")); 
        cron_parse_expr(state.Expression, &expr, &err);
#ifdef PLUGIN_078_DEBUG
        Serial.print(F("Expression:"));
        Serial.println(state.Expression);
        Serial.print(F("Errors:"));
        Serial.println(String(err));
#endif
        if (!err) 
        {
          state.LastExecution = mktime((struct tm *)&current); // Set current time;
          state.NextExecution = cron_next((cron_expr *)&expr, state.LastExecution);        
          String log = SaveCustomTaskSettings(event->TaskIndex, (byte*)&state, sizeof(state));
          if(log != F("")) 
          {
            log = String(PSTR(PLUGIN_NAME_078)) + String(F(":")) + log;
            addLog(LOG_LEVEL_ERROR, log);
          }          
          LoadTaskSettings(event->TaskIndex);
          rulesProcessing(String(F("Cron#")) + String(ExtraTaskSettings.TaskDeviceName));
        }
        else
        {
          //TODO: Notify at ui de error
          log = String(F("CRON Expression:")) + String(err);
          addLog(LOG_LEVEL_ERROR, log);        
        }
      }
      success = true;

    }

    case PLUGIN_TEN_PER_SECOND:
    {
      //code to be executed 10 times per second. Tasks which require fast response can be added here
      //be careful on what is added here. Heavy processing will result in slowing the module down!

      success = true;

    }
  }   // switch
  return success;

}     //function



//#endif
//#endif
#endif

