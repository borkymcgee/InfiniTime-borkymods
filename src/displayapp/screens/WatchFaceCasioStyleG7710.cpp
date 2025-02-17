#include "displayapp/screens/WatchFaceCasioStyleG7710.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "components/ble/weather/WeatherService.h"
#include "components/ble/MusicService.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceCasioStyleG7710*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFaceCasioStyleG7710::WatchFaceCasioStyleG7710(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificationManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::FS& filesystem,
                                                   Controllers::TouchHandler& touchHandler,
                                                   Controllers::WeatherService& weatherService,
                                                   Controllers::MusicService& musicService)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    touchHandler {touchHandler},
    weatherService {weatherService},
    musicService {musicService} {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_dot40 = lv_font_load("F:/fonts/lv_font_dots_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment40 = lv_font_load("F:/fonts/7segments_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_115.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment115 = lv_font_load("F:/fonts/7segments_115.bin");
  }

  label_battery_value = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_value, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_local_text_color(label_battery_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(label_battery_value, "00%");

  batteryIcon.Create(lv_scr_act());
  batteryIcon.SetColor(color_text);
  lv_obj_align(batteryIcon.GetObject(), label_battery_value, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text(weatherIcon, Symbols::cloudSunRain);
  // Change weather location based on time format, as there is only room to put it on the side in 12HR
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
    lv_obj_align(weatherIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 112, 72); // upper
  } else {
    lv_obj_align(weatherIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 4, 25); // lower
  }
  lv_obj_set_auto_realign(weatherIcon, true);
  if (settingsController.GetCSGWeatherStyle() == Pinetime::Controllers::Settings::CSGWeatherStyle::Off) {
    lv_obj_set_hidden(weatherIcon, true);
  } else {
    lv_obj_set_hidden(weatherIcon, false);
  }

  temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  // Change temperature location based on time format, as there is only room to put it on the side in 12HR
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
    lv_obj_align(temperature, lv_scr_act(), LV_ALIGN_CENTER, 10, -2); // above colon
    // lv_obj_align(temperature, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 4, 20);   //tucked into the 2
  } else {
    lv_obj_align(temperature, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 4, 50); // lower
  }
  if (settingsController.GetCSGWeatherStyle() == Pinetime::Controllers::Settings::CSGWeatherStyle::Off) {
    lv_obj_set_hidden(temperature, true);
  } else {
    lv_obj_set_hidden(temperature, false);
  }

  touchLockFinger = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(touchLockFinger, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(touchLockFinger, "t");
  lv_obj_align(touchLockFinger, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -120, 0);

  touchLockCross = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(touchLockCross, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(touchLockCross, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text_static(touchLockCross, Symbols::ban);
  lv_obj_align(touchLockCross, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -112, -1); //+1,-1 relative to finger

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, bleIcon, LV_ALIGN_OUT_LEFT_MID, -14, 0);
  lv_label_set_text_fmt(notificationIcon, "0");

  label_day_of_week = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_day_of_week, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 64);
  lv_obj_set_style_local_text_color(label_day_of_week, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_day_of_week, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_dot40);
  lv_label_set_text_static(label_day_of_week, "SUN");

  label_week_number = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_week_number, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 22);
  lv_obj_set_style_local_text_color(label_week_number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_week_number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_dot40);
  lv_label_set_text_static(label_week_number, "WK26");

  label_day_of_year = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_day_of_year, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 100, 30);
  lv_obj_set_style_local_text_color(label_day_of_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_day_of_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_text_static(label_day_of_year, "181-184");

  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 2);
  lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, color_text);
  lv_style_set_line_rounded(&style_line, LV_STATE_DEFAULT, true);

  lv_style_init(&style_border);
  lv_style_set_line_width(&style_border, LV_STATE_DEFAULT, 6);
  lv_style_set_line_color(&style_border, LV_STATE_DEFAULT, color_text);
  lv_style_set_line_rounded(&style_border, LV_STATE_DEFAULT, true);

  line_icons = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_icons, line_icons_points, 3);
  lv_obj_add_style(line_icons, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_icons, nullptr, LV_ALIGN_IN_TOP_RIGHT, -10, 18);

  line_day_of_week_number = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_day_of_week_number, line_day_of_week_number_points, 4);
  lv_obj_add_style(line_day_of_week_number, LV_LINE_PART_MAIN, &style_border);
  lv_obj_align(line_day_of_week_number, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 8);

  line_day_of_year = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_day_of_year, line_day_of_year_points, 3);
  lv_obj_add_style(line_day_of_year, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_day_of_year, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 60);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_text_static(label_date, "6-30");
  // nudge date over to make room for weather when in 24HR mode with weather enabled
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24 &&
      settingsController.GetCSGWeatherStyle() == Pinetime::Controllers::Settings::CSGWeatherStyle::On) {
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 118, 70); // nudge
  } else {
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 100, 70); // un-nudge
  }

  line_date = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_date, line_date_points, 3);
  lv_obj_add_style(line_date, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_date, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 100);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment115);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);

  line_time = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_time, line_time_points, 3);
  lv_obj_add_style(line_time, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_time, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -25);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time_ampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, -5);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  btnWeather = lv_btn_create(lv_scr_act(), nullptr);
  btnWeather->user_data = this;
  lv_obj_set_size(btnWeather, 160, 60);
  lv_obj_align(btnWeather, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_local_bg_opa(btnWeather, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
  lblWeather = lv_label_create(btnWeather, nullptr);
  lv_obj_set_event_cb(btnWeather, event_handler);
  lv_obj_set_hidden(btnWeather, true);
  if (settingsController.GetCSGWeatherStyle() == Pinetime::Controllers::Settings::CSGWeatherStyle::Off) {
    lv_label_set_text_static(lblWeather, "Weather: Off");
  } else {
    lv_label_set_text_static(lblWeather, "Weather: On");
  }

  btnTempUnits = lv_btn_create(lv_scr_act(), nullptr);
  btnTempUnits->user_data = this;
  lv_obj_set_size(btnTempUnits, 160, 60);
  lv_obj_align(btnTempUnits, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_bg_opa(btnTempUnits, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
  lblTempUnits = lv_label_create(btnTempUnits, nullptr);
  lv_obj_set_event_cb(btnTempUnits, event_handler);
  lv_obj_set_hidden(btnTempUnits, true);
  if (settingsController.GetTempUnits() == Controllers::Settings::TempUnits::Celcius) {
    lv_label_set_text_static(lblTempUnits, "Units: °C");
  } else {
    lv_label_set_text_static(lblTempUnits, "Units: °F");
  }

  txtMedia = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtMedia, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_align(txtMedia, heartbeatValue, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_obj_set_style_local_text_color(txtMedia, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_width(txtMedia, 80);
  lv_label_set_text_static(txtMedia, "No media playing");
  lv_obj_set_hidden(txtMedia, false);
  track = "";

  btnMedia = lv_btn_create(lv_scr_act(), nullptr);
  btnMedia->user_data = this;
  lv_obj_set_size(btnMedia, 160, 60);
  lv_obj_align(btnMedia, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnMedia, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
  lblMedia = lv_label_create(btnMedia, nullptr);
  lv_obj_set_event_cb(btnMedia, event_handler);
  lv_obj_set_hidden(btnMedia, true);
  switch (settingsController.GetCSGMediaStyle()) {
    case Pinetime::Controllers::Settings::CSGMediaStyle::Off:
      lv_label_set_text_static(lblMedia, "Media: Off");
      lv_obj_set_hidden(txtMedia, true);
      break;
    case Pinetime::Controllers::Settings::CSGMediaStyle::Artist:
      lv_label_set_text_static(lblMedia, "Media: Artist");
      break;
    case Pinetime::Controllers::Settings::CSGMediaStyle::Track:
      lv_label_set_text_static(lblMedia, "Media: Track");
      break;
    case Pinetime::Controllers::Settings::CSGMediaStyle::Album:
      lv_label_set_text_static(lblMedia, "Media: Album");
      break;
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceCasioStyleG7710::~WatchFaceCasioStyleG7710() {
  lv_task_del(taskRefresh);

  lv_style_reset(&style_line);
  lv_style_reset(&style_border);

  if (font_dot40 != nullptr) {
    lv_font_free(font_dot40);
  }

  if (font_segment40 != nullptr) {
    lv_font_free(font_segment40);
  }

  if (font_segment115 != nullptr) {
    lv_font_free(font_segment115);
  }

  lv_obj_clean(lv_scr_act());
}

bool WatchFaceCasioStyleG7710::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::LongTap && lv_obj_get_hidden(btnWeather)) {
    lv_obj_set_hidden(btnWeather, false);
    lv_obj_set_hidden(btnMedia, false);
    lv_obj_set_hidden(btnTempUnits, false);
    savedTick = lv_tick_get();
    return true;
  }
  return false;
}

void WatchFaceCasioStyleG7710::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnWeather, true);
  lv_obj_set_hidden(btnMedia, true);
  lv_obj_set_hidden(btnTempUnits, true);
}

bool WatchFaceCasioStyleG7710::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnWeather)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFaceCasioStyleG7710::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
    lv_label_set_text_fmt(label_battery_value, "%d%%", batteryPercent);
  }

  if (weatherService.GetCurrentTemperature()->timestamp != 0 && weatherService.GetCurrentClouds()->timestamp != 0 &&
      weatherService.GetCurrentPrecipitation()->timestamp != 0) {
    if (settingsController.GetTempUnits() == Controllers::Settings::TempUnits::Celcius) {
      nowTemp = (weatherService.GetCurrentTemperature()->temperature / 100); // just use temp as is
    } else {
      nowTemp = ((weatherService.GetCurrentTemperature()->temperature / 100) * 1.8 + 32); // convert to F first
    }
    clouds = (weatherService.GetCurrentClouds()->amount);
    precip = (weatherService.GetCurrentPrecipitation()->amount);
    if (nowTemp.IsUpdated()) {
      lv_label_set_text_fmt(temperature, "%d°", nowTemp.Get());
      if ((clouds <= 30) && (precip == 0)) {
        lv_label_set_text(weatherIcon, Symbols::sun);
      } else if ((clouds >= 70) && (clouds <= 90) && (precip == 1)) {
        lv_label_set_text(weatherIcon, Symbols::cloudSunRain);
      } else if ((clouds > 90) && (precip == 0)) {
        lv_label_set_text(weatherIcon, Symbols::cloud);
      } else if ((clouds > 70) && (precip >= 2)) {
        lv_label_set_text(weatherIcon, Symbols::cloudShowersHeavy);
      } else {
        lv_label_set_text(weatherIcon, Symbols::cloudSun);
      };
      lv_obj_realign(temperature);
      lv_obj_realign(weatherIcon);
    }
  } else {
    lv_label_set_text_static(temperature, "--°");
    lv_label_set_text(weatherIcon, Symbols::ban);
    lv_obj_realign(temperature);
    lv_obj_realign(weatherIcon);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(label_battery_value);
  lv_obj_realign(batteryIcon.GetObject());
  lv_obj_realign(batteryPlug);
  lv_obj_realign(bleIcon);
  lv_obj_realign(notificationIcon);

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    uint8_t notifNb = notificationManager.NbNotifications();
    lv_label_set_text_fmt(notificationIcon, "%i", notifNb);
  }

  lv_obj_set_hidden(touchLockCross, touchHandler.touchEnabled);

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[2] = "A";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_realign(label_time);

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      const char* weekNumberFormat = "%V";

      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      int dayOfYear = dateTimeController.DayOfYear();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        // 24h mode: ddmmyyyy, first DOW=Monday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", day, month);
        weekNumberFormat = "%V"; // Replaced by the week number of the year (Monday as the first day of the week) as a decimal number
                                 // [01,53]. If the week containing 1 January has four or more days in the new year, then it is considered
                                 // week 1. Otherwise, it is the last week of the previous year, and the next week is week 1. Both January
                                 // 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
      } else {
        // 12h mode: mmddyyyy, first DOW=Sunday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", month, day);
        weekNumberFormat = "%U"; // Replaced by the week number of the year as a decimal number [00,53]. The first Sunday of January is the
                                 // first day of week 1; days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
      }

      time_t ttTime =
        std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDateTime.Get()));
      tm* tmTime = std::localtime(&ttTime);

      // TODO: When we start using C++20, use std::chrono::year::is_leap
      int daysInCurrentYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 366 : 365;
      uint16_t daysTillEndOfYearNumber = daysInCurrentYear - dayOfYear;

      char buffer[8];
      strftime(buffer, 8, weekNumberFormat, tmTime);
      uint8_t weekNumber = atoi(buffer);

      lv_label_set_text_fmt(label_day_of_week, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(label_day_of_year, "%3d-%3d", dayOfYear, daysTillEndOfYearNumber);
      lv_label_set_text_fmt(label_week_number, "WK%02d", weekNumber);

      lv_obj_realign(label_day_of_week);
      lv_obj_realign(label_day_of_year);
      lv_obj_realign(label_week_number);
      lv_obj_realign(label_date);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
    lv_obj_realign(txtMedia);
    lv_obj_set_width(txtMedia, 150 - (lv_obj_get_width(stepValue)) - lv_obj_get_width(heartbeatValue));
  }

  if (!lv_obj_get_hidden(txtMedia)) {
    if (track != musicService.getTrack()) {
      track = musicService.getTrack();
      artist = musicService.getArtist();
      album = musicService.getAlbum();
      switch (settingsController.GetCSGMediaStyle()) {
        case Pinetime::Controllers::Settings::CSGMediaStyle::Artist:
          lv_label_set_text(txtMedia, artist.data());
          break;
        case Pinetime::Controllers::Settings::CSGMediaStyle::Track:
          lv_label_set_text(txtMedia, track.data());
          break;
        case Pinetime::Controllers::Settings::CSGMediaStyle::Album:
          lv_label_set_text(txtMedia, album.data());
          break;
        default:
          break;
      }
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
    lv_obj_set_width(txtMedia, 150 - (lv_obj_get_width(stepValue)) - lv_obj_get_width(heartbeatValue));
  }

  // dismiss settings menu after 3 seconds
  if (!lv_obj_get_hidden(btnMedia)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnWeather, true);
      lv_obj_set_hidden(btnMedia, true);
      lv_obj_set_hidden(btnTempUnits, true);
      savedTick = 0;
    }
  }
}

// handle settings buttons and update settings accordingly
void WatchFaceCasioStyleG7710::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    savedTick = lv_tick_get(); // reset 3 second timer to dismiss
    if (object == btnTempUnits) { // if units button, switch units
      if (settingsController.GetTempUnits() == Controllers::Settings::TempUnits::Celcius) {
        settingsController.SetTempUnits(Controllers::Settings::TempUnits::Fahrenheit);
        lv_label_set_text_static(lblTempUnits, "Units: °F");
      } else {
        settingsController.SetTempUnits(Controllers::Settings::TempUnits::Celcius);
        lv_label_set_text_static(lblTempUnits, "Units: °C");
      }
    } else if (object == btnMedia) {
      switch (settingsController.GetCSGMediaStyle()) {
        case Pinetime::Controllers::Settings::CSGMediaStyle::Off:
          lv_label_set_text_static(lblMedia, "Media: Artist");
          lv_obj_set_hidden(txtMedia, false);
          settingsController.SetCSGMediaStyle(Pinetime::Controllers::Settings::CSGMediaStyle::Artist);
          break;
        case Pinetime::Controllers::Settings::CSGMediaStyle::Artist:
          lv_label_set_text_static(lblMedia, "Media: Track");
          lv_obj_set_hidden(txtMedia, false);
          settingsController.SetCSGMediaStyle(Pinetime::Controllers::Settings::CSGMediaStyle::Track);
          break;
        case Pinetime::Controllers::Settings::CSGMediaStyle::Track:
          lv_label_set_text_static(lblMedia, "Media: Album");
          lv_obj_set_hidden(txtMedia, false);
          settingsController.SetCSGMediaStyle(Pinetime::Controllers::Settings::CSGMediaStyle::Album);
          break;
        case Pinetime::Controllers::Settings::CSGMediaStyle::Album:
          lv_label_set_text_static(lblMedia, "Media: Off");
          lv_obj_set_hidden(txtMedia, true);
          settingsController.SetCSGMediaStyle(Pinetime::Controllers::Settings::CSGMediaStyle::Off);
          break;
      }
      track = "";
    } else if (object == btnWeather) {             // if weather button pressed
      if (lv_obj_get_hidden(weatherIcon)) { // if weather hidden
        // show weather icon and temperature
        lv_obj_set_hidden(weatherIcon, false);
        lv_obj_set_hidden(temperature, false);
        // if 24HR, nudge date over to make room
        if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
          lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 118, 70); // nudge date
        }
        lv_label_set_text_static(lblWeather, "Weather: On"); // update button text
        // save to settings
        settingsController.SetCSGWeatherStyle(Controllers::Settings::CSGWeatherStyle::On);
      } else {
        // hide weather icon and temperature
        lv_obj_set_hidden(weatherIcon, true);
        lv_obj_set_hidden(temperature, true);
        if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
          // if 24HR, un-nudge date
          lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 100, 70); // un-nudge date
        }
        lv_label_set_text_static(lblWeather, "Weather: Off"); // update button text
        // save to settings
        settingsController.SetCSGWeatherStyle(Controllers::Settings::CSGWeatherStyle::Off);
      }
    }
  }
}

bool WatchFaceCasioStyleG7710::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_115.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
