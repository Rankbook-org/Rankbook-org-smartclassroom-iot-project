# Dashboard

One class dashboard for every pair's device: devices online, latest reading per device, one chart line per device, and a device picker. Open `index.html?device=pair-07` to jump straight to one device.

1. Open `index.html`.
2. Replace `SUPABASE_URL` and `SUPABASE_ANON_KEY` with the dedicated workshop project's URL and publishable (or legacy anon) key.
3. Host it (recommended) or run it locally.

**Host:** the publishable key is designed to be public, so you can put this single file on GitHub Pages, Netlify Drop or Vercel and share the URL. Students can open it on their phones.

**Local:** from this directory run

```bash
python -m http.server 8000      # Windows: py -m http.server 8000
```

and open `http://localhost:8000`.

Do not put the Supabase service-role/secret key in the browser. The dashboard only reads the workshop table, and its access depends on the SELECT policy in `supabase/schema.sql`.
