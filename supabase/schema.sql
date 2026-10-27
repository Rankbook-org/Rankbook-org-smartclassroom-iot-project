-- Smart Classroom IoT workshop schema.
-- Run only in a dedicated workshop Supabase project.
-- The public SELECT policy is for a non-sensitive classroom demo only.

CREATE TABLE IF NOT EXISTS public.sensor_readings (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    device_id TEXT NOT NULL,
    temperature NUMERIC(5,2) NOT NULL,
    humidity NUMERIC(5,2) NOT NULL,
    status TEXT NOT NULL CHECK (status IN ('NORMAL', 'WARNING')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT valid_temperature CHECK (temperature BETWEEN -40 AND 125),
    CONSTRAINT valid_humidity CHECK (humidity BETWEEN 0 AND 100)
);

ALTER TABLE public.sensor_readings ENABLE ROW LEVEL SECURITY;

-- Browser clients can read demo rows, but cannot insert/update/delete directly.
REVOKE ALL ON TABLE public.sensor_readings FROM anon, authenticated;
GRANT SELECT ON TABLE public.sensor_readings TO anon, authenticated;

DROP POLICY IF EXISTS "Public can view workshop sensor readings" ON public.sensor_readings;
CREATE POLICY "Public can view workshop sensor readings"
ON public.sensor_readings
FOR SELECT
TO anon, authenticated
USING (true);
