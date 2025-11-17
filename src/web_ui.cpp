#include "web_ui.h"
#include <ArduinoJson.h>

// Minimal HTML/JS/CSS UI embedded as a string. Sends GET requests to update/delete.
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Events Manager</title>
  <style>
    body{font-family:Arial,Helvetica,sans-serif;margin:0;padding:0;background:#f6f8fa}
    header{background:#0366d6;color:#fff;padding:12px 16px}
    main{padding:16px}
    table{width:100%;border-collapse:collapse;background:#fff}
    th,td{padding:8px;border-bottom:1px solid #eee;text-align:left}
    tr:hover{background:#fafafa}
    .btn{padding:6px 10px;border-radius:4px;border:0;cursor:pointer}
    .btn.edit{background:#28a745;color:#fff}
    .btn.del{background:#d73a49;color:#fff}
    .form-row{display:flex;gap:8px;align-items:center;margin-bottom:8px}
    .form-row input{padding:6px;flex:1}
    .modal{position:fixed;left:0;top:0;right:0;bottom:0;background:rgba(0,0,0,0.5);display:none;align-items:center;justify-content:center}
    .modal .card{background:#fff;padding:16px;border-radius:6px;width:min(720px,95%)}
  </style>
</head>
<body>
  <header><h2>Events Manager</h2></header>
  <main>
    <p>Total events: <span id="total">0</span></p>
    <table id="events"><thead><tr><th>Tag</th><th>Date</th><th>Title</th><th>Alarms</th><th>Actions</th></tr></thead><tbody></tbody></table>
    <div style="margin-top:12px">
      <button id="refresh">Refresh</button>
    </div>
  </main>

  <div class="modal" id="modal">
    <div class="card">
      <h3>Edit event</h3>
      <div class="form-row"><label style="width:80px">Tag</label><input id="m_tag" readonly></div>
      <div class="form-row"><label style="width:80px">Title</label><input id="m_title"></div>
      <div class="form-row"><label style="width:80px">Day</label><input id="m_day" type="number" min="1" max="31"></div>
      <div class="form-row"><label style="width:80px">Month</label><input id="m_month" type="number" min="1" max="12"></div>
      <div class="form-row"><label style="width:80px">Year</label><input id="m_year" type="number" min="2000" max="2099"></div>
      <div class="form-row"><label style="width:80px">Alarm0</label><input id="m_alarm0" type="number"></div>
      <div class="form-row"><label style="width:80px">Alarm1</label><input id="m_alarm1" type="number"></div>
      <div class="form-row"><label style="width:80px">Alarm2</label><input id="m_alarm2" type="number"></div>
      <div style="text-align:right;margin-top:8px">
        <button id="save" class="btn edit">Save</button>
        <button id="close" class="btn">Close</button>
      </div>
    </div>
  </div>

  <script>
  async function fetchJson(path){
    const res = await fetch(path,{cache:'no-store'});
    if(!res.ok) throw new Error('HTTP '+res.status);
    return res.json();
  }

  function render(events){
    const tbody = document.querySelector('#events tbody');
    tbody.innerHTML='';
    let total=0;
    events.forEach(ev=>{
      total++;
      const tr=document.createElement('tr');
      const date = ev.day && ev.month && ev.year ? `${String(ev.day).padStart(2,'0')}/${String(ev.month).padStart(2,'0')}/${ev.year}` : '-';
      const alarms = (ev.alarms||[]).map(a=>a.daysBefore).join(',');
      tr.innerHTML=`<td>${ev.tagId}</td><td>${date}</td><td>${ev.title||''}</td><td>${alarms}</td><td><button class="btn edit" data-tag="${ev.tagId}">Edit</button> <button class="btn del" data-tag="${ev.tagId}">Del</button></td>`;
      tbody.appendChild(tr);
    });
    document.getElementById('total').textContent = total;
    // wire buttons
    document.querySelectorAll('.btn.edit').forEach(b=>b.onclick=()=>openEditor(b.dataset.tag));
    document.querySelectorAll('.btn.del').forEach(b=>b.onclick=()=>delEvent(b.dataset.tag));
  }

  let currentEvents=[];
  async function refresh(){
    try{
      const data = await fetchJson('/api/events');
      currentEvents = data.events || [];
      render(currentEvents);
    }catch(e){console.error(e);alert('Failed to load events');}
  }

  function openEditor(tag){
    const ev = currentEvents.find(e=>String(e.tagId)===String(tag));
    if(!ev) return;
    document.getElementById('m_tag').value = ev.tagId;
    document.getElementById('m_title').value = ev.title||'';
    document.getElementById('m_day').value = ev.day||'';
    document.getElementById('m_month').value = ev.month||'';
    document.getElementById('m_year').value = ev.year||'';
    document.getElementById('m_alarm0').value = (ev.alarms&&ev.alarms[0])?ev.alarms[0].daysBefore:'';
    document.getElementById('m_alarm1').value = (ev.alarms&&ev.alarms[1])?ev.alarms[1].daysBefore:'';
    document.getElementById('m_alarm2').value = (ev.alarms&&ev.alarms[2])?ev.alarms[2].daysBefore:'';
    document.getElementById('modal').style.display='flex';
  }

  function closeEditor(){ document.getElementById('modal').style.display='none'; }

  async function saveEvent(){
    const tag = document.getElementById('m_tag').value;
    const title = encodeURIComponent(document.getElementById('m_title').value || '');
    const day = document.getElementById('m_day').value;
    const month = document.getElementById('m_month').value;
    const year = document.getElementById('m_year').value;
    const a0 = document.getElementById('m_alarm0').value;
    const a1 = document.getElementById('m_alarm1').value;
    const a2 = document.getElementById('m_alarm2').value;
    // Build query string for GET update
    const qs = `?tagId=${tag}&title=${title}` + (day?`&day=${day}`:'') + (month?`&month=${month}`:'') + (year?`&year=${year}`:'') + (a0?`&alarm0=${a0}`:'') + (a1?`&alarm1=${a1}`:'') + (a2?`&alarm2=${a2}`:'');
    const res = await fetch('/api/event/update'+qs);
    if(res.ok){ closeEditor(); refresh(); } else { alert('Update failed'); }
  }

  async function delEvent(tag){ if(!confirm('Delete event '+tag+'?')) return; const res = await fetch('/api/event/delete?tagId='+tag); if(res.ok) refresh(); else alert('Delete failed'); }

  document.getElementById('refresh').addEventListener('click', refresh);
  document.getElementById('close').addEventListener('click', closeEditor);
  document.getElementById('save').addEventListener('click', saveEvent);

  refresh();
  </script>
</body>
</html>
)rawliteral";

void setupWebUiRoutes(AsyncWebServer &server, EventList &events)
{
    // Serve main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/html", INDEX_HTML);
    });

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
