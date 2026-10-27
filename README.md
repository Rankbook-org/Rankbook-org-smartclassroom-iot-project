# Smart Classroom IoT Monitor
### Student Workshop Project — ESP32 + DHT22 + Supabase + Web Dashboard

This repository is a stage-by-stage learning project for the workshop. Students can complete one stage at a time and compare their work with the next stage.

> **Important:** This is a learning/demo project. Do not use the demo token pattern or public read policy for a production system. Never commit Supabase service-role keys, passwords, or real secrets to GitHub.

## What you will build

- Simulate an ESP32 in [Wokwi](https://wokwi.com/)
- Read temperature and humidity from a DHT22 sensor
- Turn an LED on when temperature is **30°C or higher**
- Connect to the simulated Wi-Fi network
- Send a JSON reading over HTTPS to a Supabase Edge Function
- Store readings in a PostgreSQL table
- Display readings on a browser dashboard with a chart

## Project structure

```text
smart-classroom-iot-workshop/
├── README.md
├── .gitignore
├── wokwi/
│   ├── diagram.json
│   ├── libraries.txt
│   ├── stage-01-sensor-led/
│   │   └── sketch.ino
│   ├── stage-02-wifi/
│   │   └── sketch.ino
│   ├── stage-03-http-echo/
│   │   └── sketch.ino
│   └── stage-04-supabase/
│       └── sketch.ino
├── supabase/
│   ├── schema.sql
│   ├── config.toml
│   └── functions/
│       └── ingest-reading/
│           └── index.ts
└── dashboard/
    ├── index.html
    └── README.md
```


## Stage 0 — Prepare the tools

**Before the workshop (students):** create a free Wokwi account (Google or GitHub sign-in). Without it you cannot save projects, and work is lost over the lunch break.

**Recommended (instructor):** publish one public Wokwi project per stage and share the links. Students open a link and click **Save a copy**. This avoids copy-paste mistakes with three separate files.

Manual setup, if you are not using shared links:

1. Open [Wokwi ESP32 project](https://wokwi.com/projects/new/esp32).
2. Copy `wokwi/diagram.json` into the Wokwi diagram editor.
3. Copy `wokwi/libraries.txt` into the Wokwi Libraries panel (or create the file in the project). Library names must match the Arduino Library Manager exactly: the DHTesp library is listed as `DHT sensor library for ESPx`.
4. Copy one stage's `sketch.ino` into Wokwi's `sketch.ino`.
5. Start with Stage 1. Do not jump to Stage 4 until earlier stages work.

## Wiring

| Component | Pin | ESP32 |
|---|---|---|
| DHT22 | VCC | 3V3 |
| DHT22 | DATA | GPIO 15 |
| DHT22 | GND | GND |
| LED anode (+) | Through 220Ω resistor | GPIO 18 |
| LED cathode (-) | GND | GND |

The diagram in `wokwi/diagram.json` shows this wiring. If you rebuild the circuit manually, make sure the LED has a series resistor.

## Stage 1 — Sensor + LED

**File:** `wokwi/stage-01-sensor-led/sketch.ino`

Goal:
- Read temperature and humidity.
- Print the values in Serial Monitor.
- Turn LED on and show `WARNING` when temperature is at least 30°C.
- Otherwise turn LED off and show `NORMAL`.

Expected serial output (values vary):
```text
Temperature: 26.40 C
Humidity: 54.00 %
Status: NORMAL
```

Test by changing the DHT22 temperature in Wokwi. Try values below and above 30°C.

## Stage 2 — Connect to Wi-Fi

**File:** `wokwi/stage-02-wifi/sketch.ino`

Wokwi's simulated Wi-Fi credentials are:
- SSID: `Wokwi-GUEST`
- Password: empty
- Channel: 6

Expected output includes:
```text
Connecting to WiFi...
WiFi connected
IP address: ...
```

## Stage 3 — Send JSON to an HTTP test endpoint

**File:** `wokwi/stage-03-http-echo/sketch.ino`

This stage sends a JSON payload to `https://httpbin.org/post`. The service echoes the request back; it does **not** save readings to your database.

Set `DEVICE_ID` at the top of the sketch to your pair's ID (for example `pair-07`). You will use the same ID in Stage 4.

Alternatives if `httpbin.org` is slow or returns 5xx:
- `https://postman-echo.com/post`: same idea, echoes the request.
- [webhook.site](https://webhook.site): you get a unique URL and can **watch each request arrive** in the browser, with headers and body. This is the best way to see what an HTTP request actually is.

The example uses `setInsecure()` only to keep the classroom HTTPS demonstration simple. It skips certificate validation and is **not safe for production**.

## Stage 4 — Save readings in Supabase

### A. Create a separate Supabase project

Create a new project for this workshop, for example `smart-classroom-iot`. Do not use an existing production project.

### B. Create the table

Open Supabase **SQL Editor** and run `supabase/schema.sql`.

This demo table allows anonymous browser reads so the dashboard can work without student login. Only put non-sensitive workshop data in it. The policy is intentionally broad for a classroom demo.

### C. Deploy the Edge Function

The function source is `supabase/functions/ingest-reading/index.ts`.

In Supabase Dashboard:
1. Open **Edge Functions** and create a function named `ingest-reading`.
2. Paste the function code.
3. Set a function secret named `WORKSHOP_INGEST_TOKEN` to a long random demo token.
4. Deploy the function.
5. In the function's settings, **turn OFF "Enforce JWT verification"** (Verify JWT). The ESP32 sends no `Authorization` header, so the gateway will reject every request with 401 if this is on. The function checks `x-workshop-token` itself instead. (`supabase/config.toml` already sets `verify_jwt = false` for CLI deploys.)
6. Test the function before connecting Wokwi:

```bash
curl -i -X POST "https://YOUR_PROJECT_REF.supabase.co/functions/v1/ingest-reading" \
  -H "Content-Type: application/json" \
  -H "x-workshop-token: YOUR_WORKSHOP_TOKEN" \
  -d '{"device_id":"instructor-test","temperature":27.5,"humidity":55}'
```

You should get `HTTP/2 201` with the saved row. Then run the Stage 4 sketch in Wokwi and confirm that it also gets 201. Test from Wokwi too, not only from curl.

The function uses Supabase's server-side `SUPABASE_URL` and `SUPABASE_SERVICE_ROLE_KEY` environment variables. Never copy the service-role key into Wokwi or the browser. If your project has **legacy API keys disabled** (new `sb_publishable_` / `sb_secret_` keys only), confirm that the insert still succeeds. If it returns 500 "Could not save reading", check the function logs.

### D. Configure the Wokwi sketch (every pair)

All pairs send to the **instructor's shared workshop project**. Open `wokwi/stage-04-supabase/sketch.ino` and set:
- `SUPABASE_PROJECT_URL`: the project URL your instructor gives you, e.g. `https://your-project-ref.supabase.co`
- `WORKSHOP_TOKEN`: the temporary workshop token your instructor gives you
- `DEVICE_ID`: your pair's unique ID, e.g. `pair-07` (letters, numbers, `_`, `-`; max 64)

The function URL is:
```text
https://YOUR_PROJECT_REF.supabase.co/functions/v1/ingest-reading
```

**Why a shared token is acceptable here:** anyone who sees the token can insert readings into the workshop table. The risk is limited to junk rows in a throwaway project. The function validates every field, the table holds no sensitive data, and browsers cannot write directly. Give out the token on the day only (on the projector or whiteboard, not in a public repo), and **rotate or delete it right after the workshop**. A real deployment would give each device its own credential.

**Optional homework for strong students:** create your own Supabase project and repeat Stages 4–5 end to end. Don't do this live: signup, email verification and project provisioning take 15+ minutes.

## Stage 5 — Open the class dashboard

The dashboard is one page for the whole class. It shows:
- **Devices online**: pairs that sent a reading in the last 60 seconds
- **Class overview**: the latest reading per device (click a row to focus on that device)
- **Temperature history**: one chart line per device
- **Device picker**: or open `...?device=pair-07` to jump straight to your device

**Recommended: host it once and share the URL.** The publishable/anon key is designed to be public, so the page can be hosted anywhere: GitHub Pages, Netlify Drop, or Vercel. Students can open it on their phones and the instructor shows it on the projector. No local Python is needed.

Configure (instructor, once):
1. Edit `dashboard/index.html`.
2. Set `SUPABASE_URL` to the workshop project URL.
3. Set `SUPABASE_ANON_KEY` to the project's publishable (or legacy anon) key.
4. Do **not** use the service-role/secret key.

Run locally instead (optional):

```bash
cd dashboard
python -m http.server 8000      # Windows: py -m http.server 8000
```

Then open `http://localhost:8000`. If you open the HTML file directly with `file://` and it fails, use this local server.

The dashboard refreshes every 15 seconds to avoid unnecessary requests.

## Challenges — pick your level

Only start these once the base flow works. Change one thing, re-run, and capture evidence.

| Level | Challenge |
|---|---|
| 🥉 Bronze | Change the warning threshold from 30°C to 28°C, **or** change your `DEVICE_ID` and find it on the dashboard. |
| 🥈 Silver | Add a humidity warning rule and explain your choice, **or** add **hysteresis**: LED ON at ≥ 30°C, OFF only below 29°C, so it doesn't flicker at the boundary. |
| 🥇 Gold | Switch the dashboard from polling to Supabase Realtime; **or** buffer readings on the ESP32 while Wi-Fi is down and send them when it reconnects; **or** add per-device rate limiting to the Edge Function. |

Discussion question: the device and the server both compute `status`. Which one should the dashboard trust, and why?

## Troubleshooting

- **`DHTesp.h: No such file or directory`:** `libraries.txt` must contain `DHT sensor library for ESPx` (the Library Manager name), not `DHTesp`.
- **ArduinoJson errors about `JsonDocument`:** the sketches use ArduinoJson v7. Make sure `libraries.txt` contains `ArduinoJson` with no old version pinned.
- **DHT values are NaN:** confirm GPIO 15 is the data pin (labelled `SDA` on the Wokwi DHT22 part) and the DHT22 library is listed.
- **LED does not turn on:** test temperature at 30°C or higher; check GPIO 18 and LED polarity.
- **Wi-Fi does not connect:** use `Wokwi-GUEST`, blank password, channel 6.
- **HTTP test fails:** check internet access and the endpoint URL. `httpbin.org` is only an echo test.
- **Supabase returns 401/403:** if the body says `Unauthorized workshop token.`, check the token and the `x-workshop-token` header. Any other 401 means the gateway rejected the request: turn OFF "Enforce JWT verification" for the function.
- **Supabase returns 400 about `device_id`:** use only letters, numbers, `_` and `-` (no spaces).
- **Supabase returns 400:** check JSON property names and numeric ranges.
- **Dashboard shows no rows:** confirm the function successfully inserted a row, table name is `sensor_readings`, and the public SELECT policy is enabled.
- **Browser import fails:** run `python -m http.server 8000`; check venue internet access for the CDN.



## License

For educational use. Review and adapt security settings before any real deployment.
