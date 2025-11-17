#include "web_ui.h"
#include <ArduinoJson.h>

EventListWebUi:: EventListWebUi()
{
}

void EventListWebUi::begin(AsyncWebServer &server, EventList &events)
{
  _server = &server;
  _events = &events;
  setupWebUiRoutes(server, events);

}

void  EventListWebUi::setupWebUiRoutes(AsyncWebServer &server, EventList &events)
{
    // Return JSON containing all events
    server.on("/api/events", HTTP_GET, [&events](AsyncWebServerRequest *req){
        String j = events.toJson();
        req->send(200, "application/json", j);
    });

    // Delete event by tagId (query)
    server.on("/api/event/delete", HTTP_GET, [&events](AsyncWebServerRequest *req){
        if (!req->hasParam("tagId")) { req->send(400, "text/plain", "missing tagId"); return; }
        String t = req->getParam("tagId")->value();
        uint32_t tag = (uint32_t) strtoul(t.c_str(), nullptr, 10);
        if (events.remove(tag)) req->send(200, "text/plain", "ok"); else req->send(404, "text/plain", "not found");
    });

    // Update event via query params: tagId (required), optional title, day, month, year, alarm0/1/2
    server.on("/api/event/update", HTTP_GET, [&events](AsyncWebServerRequest *req){
        if (!req->hasParam("tagId")) { req->send(400, "text/plain", "missing tagId"); return; }
        String t = req->getParam("tagId")->value();
        uint32_t tag = (uint32_t) strtoul(t.c_str(), nullptr, 10);
        // title
        if (req->hasParam("title")) {
            String title = req->getParam("title")->value();
            // decode URI component
            // simple replace for + and %20
            events.setTitle(tag, title);
        }
        // date
        if (req->hasParam("day") && req->hasParam("month") && req->hasParam("year")) {
            uint8_t day = (uint8_t) atoi(req->getParam("day")->value().c_str());
            uint8_t month = (uint8_t) atoi(req->getParam("month")->value().c_str());
            uint16_t year = (uint16_t) atoi(req->getParam("year")->value().c_str());
            events.updateDate(tag, day, month, year);
        }
        // alarms
        for (int i = 0; i < (int)Event::MAX_ALARMS; ++i) {
            String name = String("alarm") + String(i);
            if (req->hasParam(name.c_str())) {
                int val = atoi(req->getParam(name.c_str())->value().c_str());
                events.setAlarm(tag, i, (int16_t)val);
            }
        }

        req->send(200, "text/plain", "ok");
    });
}

void EventListWebUi::notifyWebUiClients(void)
{
    String json = _events->toJson();
    //_server->sendNotify("events_update", json);


}
