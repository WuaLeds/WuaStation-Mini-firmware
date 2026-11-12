#include "providers/weather/OpenMeteoWeatherProvider.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ctype.h>
#include "config/Config.h"
#include "core/Logger.h"
#include "display/TextUtil.h"
#include "net/HttpJson.h"

void OpenMeteoWeatherProvider::configure(const char *city, bool hasCoordinates,
                                         float latitude, float longitude)
{
    _hasCoordinates = hasCoordinates;
    _latitude = latitude;
    _longitude = longitude;

    // "Madrid,ES" -> name "Madrid" + country filter "ES".
    strlcpy(_city, city ? city : "", sizeof(_city));
    _country[0] = '\0';
    char *comma = strchr(_city, ',');
    if (comma)
    {
        *comma = '\0';
        const char *cc = comma + 1;
        while (*cc == ' ')
            ++cc;
        if (isalpha(static_cast<unsigned char>(cc[0])) && isalpha(static_cast<unsigned char>(cc[1])) && cc[2] == '\0')
        {
            _country[0] = static_cast<char>(toupper(static_cast<unsigned char>(cc[0])));
            _country[1] = static_cast<char>(toupper(static_cast<unsigned char>(cc[1])));
            _country[2] = '\0';
        }
    }
    strlcpy(_place, _hasCoordinates ? "" : _city, sizeof(_place));
}

uint32_t OpenMeteoWeatherProvider::run()
{
    if (!_hasCoordinates && !geocode())
        return Config::kWeatherRetryMs;
    return fetchCurrent() ? Config::kWeatherPollMs : Config::kWeatherRetryMs;
}

bool OpenMeteoWeatherProvider::geocode()
{
    String url = "https://geocoding-api.open-meteo.com/v1/search?count=1&format=json&name=";
    url += TextUtil::urlEncode(_city);
    if (_country[0])
    {
        url += "&countryCode=";
        url += _country;
    }

    JsonDocument filter;
    filter["results"][0]["latitude"] = true;
    filter["results"][0]["longitude"] = true;
    filter["results"][0]["name"] = true;

    JsonDocument doc;
    if (HttpJson::get(url, doc, &filter) != HTTP_CODE_OK)
        return false;

    JsonObjectConst result = doc["results"][0];
    if (result.isNull() || result["latitude"].isNull() || result["longitude"].isNull())
    {
        LOG_WARN("Weather: city '%s' not found", _city);
        return false;
    }

    _latitude = result["latitude"].as<float>();
    _longitude = result["longitude"].as<float>();
    strlcpy(_place, result["name"] | _city, sizeof(_place));
    _hasCoordinates = true;
    LOG_INFO("Weather: '%s' -> %.4f, %.4f", _place, _latitude, _longitude);
    return true;
}

bool OpenMeteoWeatherProvider::fetchCurrent()
{
    char url[200];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,relative_humidity_2m,weather_code,is_day&timezone=auto",
             _latitude, _longitude);

    JsonDocument filter;
    filter["current"]["temperature_2m"] = true;
    filter["current"]["relative_humidity_2m"] = true;
    filter["current"]["weather_code"] = true;
    filter["current"]["is_day"] = true;

    JsonDocument doc;
    if (HttpJson::get(String(url), doc, &filter) != HTTP_CODE_OK)
        return false;

    JsonObjectConst current = doc["current"];
    if (current.isNull() || current["temperature_2m"].isNull() || current["weather_code"].isNull())
        return false;

    const uint16_t code = current["weather_code"].as<uint16_t>();
    _work.valid = true;
    _work.temperatureC = current["temperature_2m"].as<float>();
    _work.humidity = static_cast<uint8_t>(constrain(current["relative_humidity_2m"] | 0, 0, 100));
    _work.weatherCode = code;
    _work.condition = conditionFromWmo(code);
    _work.isDay = (current["is_day"] | 1) != 0;
    strlcpy(_work.place, _place, sizeof(_work.place));
    _work.updatedMs = millis();

    _shared.write(_work);
    _hasData.store(true);
    LOG_DEBUG("Weather: %.1f C, %u %%, code %u, %s", _work.temperatureC, _work.humidity, code,
              _work.isDay ? "day" : "night");
    return true;
}

// WMO weather interpretation codes, as used by Open-Meteo.
WeatherCondition OpenMeteoWeatherProvider::conditionFromWmo(uint16_t code)
{
    switch (code)
    {
    case 0:
        return WeatherCondition::Clear;
    case 1:
    case 2:
        return WeatherCondition::PartlyCloudy;
    case 3:
        return WeatherCondition::Cloudy;
    case 45:
    case 48:
        return WeatherCondition::Fog;
    case 51: case 53: case 55: case 56: case 57: // drizzle
    case 61: case 63: case 65: case 66: case 67: // rain
    case 80: case 81: case 82:                   // showers
        return WeatherCondition::Rain;
    case 71: case 73: case 75: case 77: // snow
    case 85: case 86:                   // snow showers
        return WeatherCondition::Snow;
    case 95: case 96: case 99:
        return WeatherCondition::Thunderstorm;
    default:
        return WeatherCondition::Unknown;
    }
}
