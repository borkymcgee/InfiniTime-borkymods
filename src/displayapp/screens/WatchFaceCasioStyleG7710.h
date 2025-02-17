#pragma once

#include <displayapp/screens/BatteryIcon.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "touchhandler/TouchHandler.h"
#include "components/ble/weather/WeatherService.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class MusicService;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceCasioStyleG7710 : public Screen {
      public:
        WatchFaceCasioStyleG7710(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController& heartRateController,
                                 Controllers::MotionController& motionController,
                                 Controllers::FS& filesystem,
                                 Controllers::TouchHandler& touchHandler,
                                 Controllers::WeatherService& weather,
                                 Controllers::MusicService& musicService);
        ~WatchFaceCasioStyleG7710() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

        void Refresh() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<int16_t> nowTemp {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;

        uint32_t savedTick = 0;

        int16_t clouds = 0;
        int16_t precip = 0;

        lv_point_t line_icons_points[3] {{0, 5}, {117, 5}, {122, 0}};
        lv_point_t line_day_of_week_number_points[4] {{0, 0}, {100, 0}, {95, 95}, {0, 95}};
        lv_point_t line_day_of_year_points[3] {{0, 5}, {130, 5}, {135, 0}};
        lv_point_t line_date_points[3] {{0, 5}, {135, 5}, {140, 0}};
        lv_point_t line_time_points[3] {{0, 0}, {230, 0}, {235, 5}};

        lv_color_t color_text = lv_color_hex(0x98B69A);

        lv_style_t style_line;
        lv_style_t style_border;

        lv_obj_t* btnWeather;
        lv_obj_t* btnTempUnits;
        lv_obj_t* lblTempUnits;
        lv_obj_t* lblWeather;
        lv_obj_t* btnMedia;
        lv_obj_t* lblMedia;
        lv_obj_t* label_time;
        lv_obj_t* line_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        lv_obj_t* line_date;
        lv_obj_t* label_day_of_week;
        lv_obj_t* label_week_number;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;
        lv_obj_t* line_day_of_week_number;
        lv_obj_t* label_day_of_year;
        lv_obj_t* line_day_of_year;
        lv_obj_t* backgroundLabel;
        lv_obj_t* bleIcon;
        lv_obj_t* touchLockFinger;
        lv_obj_t* touchLockCross;
        lv_obj_t* batteryPlug;
        lv_obj_t* label_battery_value;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* line_icons;
        lv_obj_t* txtMedia;

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::TouchHandler& touchHandler;
        Controllers::WeatherService& weatherService;
        Controllers::MusicService& musicService;

        std::string artist = "no artist";
        std::string album = "no album";
        std::string track = "no track";

        void CloseMenu();

        lv_task_t* taskRefresh;
        lv_font_t* font_dot40 = nullptr;
        lv_font_t* font_segment40 = nullptr;
        lv_font_t* font_segment115 = nullptr;
      };
    }
  }
}
