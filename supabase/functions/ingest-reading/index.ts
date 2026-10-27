import { createClient } from "https://esm.sh/@supabase/supabase-js@2";

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Headers": "authorization, x-client-info, apikey, content-type, x-workshop-token",
  "Access-Control-Allow-Methods": "POST, OPTIONS",
};

function jsonResponse(body: unknown, status = 200) {
  return new Response(JSON.stringify(body), {
    status,
    headers: { ...corsHeaders, "Content-Type": "application/json" },
  });
}

Deno.serve(async (req: Request) => {
  if (req.method === "OPTIONS") {
    return new Response("ok", { headers: corsHeaders });
  }

  if (req.method !== "POST") {
    return jsonResponse({ error: "Method not allowed. Use POST." }, 405);
  }

  const expectedToken = Deno.env.get("WORKSHOP_INGEST_TOKEN");
  const receivedToken = req.headers.get("x-workshop-token");

  if (!expectedToken || !receivedToken || receivedToken !== expectedToken) {
    return jsonResponse({ error: "Unauthorized workshop token." }, 401);
  }

  let payload: Record<string, unknown>;
  try {
    payload = await req.json();
  } catch {
    return jsonResponse({ error: "Request body must be valid JSON." }, 400);
  }

  if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
    return jsonResponse({ error: "Request body must be a JSON object." }, 400);
  }

  const deviceId = payload.device_id;
  const temperature = payload.temperature;
  const humidity = payload.humidity;

  if (
    typeof deviceId !== "string" ||
    !/^[a-zA-Z0-9_-]{1,64}$/.test(deviceId)
  ) {
    return jsonResponse({
      error: "device_id must be 1-64 characters: letters, numbers, underscore, or hyphen.",
    }, 400);
  }

  if (
    typeof temperature !== "number" ||
    !Number.isFinite(temperature) ||
    temperature < -40 ||
    temperature > 125
  ) {
    return jsonResponse({ error: "temperature must be a number from -40 to 125." }, 400);
  }

  if (
    typeof humidity !== "number" ||
    !Number.isFinite(humidity) ||
    humidity < 0 ||
    humidity > 100
  ) {
    return jsonResponse({ error: "humidity must be a number from 0 to 100." }, 400);
  }

  // Calculate status on the server; do not trust a status supplied by the device.
  const status = temperature >= 30 ? "WARNING" : "NORMAL";

  const supabaseUrl = Deno.env.get("SUPABASE_URL");
  const serviceRoleKey = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");

  if (!supabaseUrl || !serviceRoleKey) {
    return jsonResponse({ error: "Server configuration is missing." }, 500);
  }

  const supabase = createClient(supabaseUrl, serviceRoleKey, {
    auth: { persistSession: false, autoRefreshToken: false },
  });

  const { data, error } = await supabase
    .from("sensor_readings")
    .insert({
      device_id: deviceId,
      temperature,
      humidity,
      status,
    })
    .select("id, device_id, temperature, humidity, status, created_at")
    .single();

  if (error) {
    console.error("Insert failed:", error.message);
    return jsonResponse({ error: "Could not save reading." }, 500);
  }

  return jsonResponse({ message: "Reading saved.", reading: data }, 201);
});
