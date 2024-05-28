#include <toneAC.h>
#include <NewPing.h>
#define LOG_OUTPUT 0
#define LINEAR_SCALE 0

#define VOLUME_TRIG ((uint8_t)3)
#define VOLUME_ECHO ((uint8_t)4)
#define MIN_VOL_DISTANCE ((unsigned int)5) // cm
#define MAX_VOL_DISTANCE ((unsigned int)55)
#define RANGE_VOL_DISTANCE ((unsigned int)(MAX_VOL_DISTANCE - MIN_VOL_DISTANCE))
#define MIN_SCALED_VOL ((unsigned int)1)
#define MAX_SCALED_VOL ((unsigned int)10)
#define RANGE_SCALED_VOL (MAX_SCALED_VOL - MIN_SCALED_VOL)

unsigned int scale_volume(unsigned int vol_distance)
{
  /*
  NewPing.ping_cm() returns NO_ECHO if the measured distance is greater than the maximum specified distance.
  Use 4 states:
    NO_ECHO -- turn off any tone.
    NO_ECHO < distance <= MIN_VOL_DISTANCE -- proceed as if distance = MIN_VOL_DISTANCE
    distance > MAX_VOL_DISTANCE -- proceed as if distance = MAX_VOL_DISTANCE (MAX_VOL_DISTANCE and the maximum  NewPing.ping_cm() need not be the same).
    MIN_VOL_DISTANCE <= distance <=MAX_VOL_DISTANCE -- map onto a volume value from 1 to 10 (inclusive).

  */

  if (vol_distance == NO_ECHO)
  {
#if LOG_OUTPUT
    Serial.print("scale_volume(");
    Serial.print(vol_distance);
    Serial.print(") -> ");
    Serial.println(NO_ECHO);
#endif
    return NO_ECHO;
  }

  if (vol_distance <= MIN_VOL_DISTANCE)
  {
#if LOG_OUTPUT
    Serial.print("scale_volume(");
    Serial.print(vol_distance);
    Serial.print(") -> ");
    Serial.println(MIN_SCALED_VOL);
#endif
    return MIN_SCALED_VOL;
  }

  if (vol_distance >= MAX_VOL_DISTANCE)
  {
#if LOG_OUTPUT
    Serial.print("scale_volume(");
    Serial.print(vol_distance);
    Serial.print(") -> ");
    Serial.println(MAX_SCALED_VOL);
#endif
    return MAX_SCALED_VOL;
  }

  unsigned int r = ((unsigned int)(MIN_SCALED_VOL + (vol_distance - MIN_VOL_DISTANCE) * RANGE_SCALED_VOL / RANGE_VOL_DISTANCE));
#if LOG_OUTPUT
  Serial.print("scale_volume(");
  Serial.print(vol_distance);
  Serial.print(") ->  ");
  Serial.print(r);
  Serial.print(" = ((unsigned int)(");
  Serial.print(MIN_SCALED_VOL);
  Serial.print(" + (");
  Serial.print(vol_distance);
  Serial.print(" - ");
  Serial.print(MIN_VOL_DISTANCE);
  Serial.print(") * ");
  Serial.print(RANGE_SCALED_VOL);
  Serial.print(" / ");
  Serial.print(RANGE_VOL_DISTANCE);
  Serial.println(" ))");
#endif
  return r;
}

#define TONE_TRIG ((uint8_t)5)
#define TONE_ECHO ((uint8_t)6)

/*
 * Indices into freqtable to set the min/max frequency range.
 */
#define MIN_SCALED_TONE ((size_t)24)
#define MAX_SCALED_TONE ((size_t)48)
// #define MIN_SCALED_TONE ((size_t)0)
// #define MAX_SCALED_TONE ((size_t)24)
#define RANGE_SCALED_TONE (MAX_SCALED_TONE - MIN_SCALED_TONE)

#define MIN_TONE_DISTANCE ((unsigned int)5)                                                                 // cm
#define MAX_TONE_DISTANCE ((unsigned int)(MIN_TONE_DISTANCE + 2 * (MAX_SCALED_TONE - MIN_SCALED_TONE + 1))) // cm
#define RANGE_TONE_DISTANCE ((unsigned int)(MAX_TONE_DISTANCE - MIN_TONE_DISTANCE))
/*
 * Table of piano notes to frequencies (A0 to C8)
 */
#define FREQTABLE_LEN ((size_t)88)
const float freqtable[] = {
    27, 29, 30, 32, 34, 36, 38, 41, 43, 46, 48, 51, 55, 58, 61, 65, 69, 73, 77, 82, 87, 92, 97, 103, 110, 116, 123, 130, 138, 146, 155, 164, 174, 184, 195, 207, 220, 233, 246, 261, 277, 293, 311, 329, 349, 369, 391, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987, 1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975, 2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951, 4186};

const unsigned int min_scaled_tone = freqtable[MIN_SCALED_TONE];
const unsigned int max_scaled_tone = freqtable[MAX_SCALED_TONE];
const unsigned int range_scaled_tone = max_scaled_tone - min_scaled_tone;
/*
 * Rather than leaving it to play forever, set a quiescent timer.
 * If nothing changes over that period of time let it time out.
 */
const unsigned int tone_duration = 0;

#if LINEAR_SCALE
#define MIN_DELTA_TONE 50
#define TONE_CHANGED_ENOUGH(a, b) (abs((a) - (b)) > MIN_DELTA_TONE)
#else
#define TONE_CHANGED_ENOUGH(a, b) ((a) != (b))
#endif

unsigned int get_frequency(unsigned int tone_distance)
{

  /*
  NewPing.ping_cm() returns NO_ECHO if the measured distance is greater than the maximum specified distance.
  Use 4 states:
    NO_ECHO -- turn off any tone.
    NO_ECHO < tone_distance <= MIN_TONE_DISTANCE -- proceed as if tone_distance = MIN_VOL_DISTANCE
    tone_distance > MAX_VOL_DISTANCE -- proceed as if tone_distance = MAX_VOL_DISTANCE (MAX_VOL_DISTANCE and the maximum  NewPing.ping_cm() need not be the same).
    MIN_VOL_DISTANCE <= tone_distance <=MAX_VOL_DISTANCE:
      If LINEAR_SCALE is set then scale tone_distance to be within the given min/max frequency.
      If LINEAR_SCALE is NOT set then scale tone_distance to be within the given min/max freqtable indices and return the freqeuncy given by that index.
  */
  if (tone_distance == NO_ECHO)
  {
#if LOG_OUTPUT
    Serial.print("get_frequency(");
    Serial.print(tone_distance);
    Serial.print(") -> ");
    Serial.println(NO_ECHO);
#endif
    return NO_ECHO;
  }

  if (tone_distance <= MIN_TONE_DISTANCE)
  {
#if LOG_OUTPUT
    Serial.print("get_frequency(");
    Serial.print(tone_distance);
    Serial.print(") -> ");
    Serial.println(min_scaled_tone);
#endif
    return min_scaled_tone;
  }
  if (tone_distance >= MAX_TONE_DISTANCE)
  {
#if LOG_OUTPUT
    Serial.print("get_frequency(");
    Serial.print(tone_distance);
    Serial.print(") -> ");
    Serial.println(max_scaled_tone);
#endif
    return max_scaled_tone;
  }

#if LINEAR_SCALE
  unsigned int fz = ((unsigned int)(min_scaled_tone + (tone_distance - MIN_TONE_DISTANCE) * range_scaled_tone / RANGE_TONE_DISTANCE));
#if LOG_OUTPUT
  Serial.print("get_frequency[linear](");
  Serial.print(tone_distance);
  Serial.print(") -> ");
  Serial.print(fz);
  Serial.print(" = ((unsigned int)");
  Serial.print(min_scaled_tone);
  Serial.print(" + (");
  Serial.print(tone_distance);
  Serial.print(" - ");
  Serial.print(MIN_TONE_DISTANCE);
  Serial.print(") * ");
  Serial.print(range_scaled_tone);
  Serial.print(" / ");
  Serial.print(RANGE_TONE_DISTANCE);
  Serial.println("))");
#endif
  return fz;
#else
  unsigned int i = ((unsigned int)(MIN_SCALED_TONE + (tone_distance - MIN_TONE_DISTANCE) * RANGE_SCALED_TONE / RANGE_TONE_DISTANCE));
#if LOG_OUTPUT
  Serial.print("get_frequency[piano](");
  Serial.print(tone_distance);
  Serial.print(") -> ");
  Serial.print(i);
  Serial.print(" = ((unsigned int)");
  Serial.print(MIN_SCALED_TONE);
  Serial.print(" + (");
  Serial.print(tone_distance);
  Serial.print(" - ");
  Serial.print(MIN_TONE_DISTANCE);
  Serial.print(") * ");
  Serial.print(RANGE_SCALED_TONE);
  Serial.print(" / ");
  Serial.print(RANGE_TONE_DISTANCE);
  Serial.println("))");
  Serial.print("freqtable[");
  Serial.print(i);
  Serial.print("] = ");
  Serial.println(freqtable[i]);
#endif
  return freqtable[i];
#endif
}

NewPing volume_sonar = NewPing(VOLUME_TRIG, VOLUME_ECHO, MAX_VOL_DISTANCE);
NewPing tone_sonar = NewPing(TONE_TRIG, TONE_ECHO, MAX_TONE_DISTANCE);

/*
 * Expect a minimum delay of 29ms per sonar.
 */
#if LOG_OUTPUT
const unsigned int loop_delay = 200;
#else
const unsigned int loop_delay = 60;
#endif

void setup()
{
#if LOG_OUTPUT
  Serial.begin(9600);
#endif
  noToneAC();
}

unsigned int new_tone = 0;
unsigned int cur_tone = 0;
uint8_t cur_volume = 0;
uint8_t new_volume = 0;

void loop()
{
  delay(loop_delay);
  new_tone = get_frequency(tone_sonar.ping_cm());
  new_volume = scale_volume(volume_sonar.ping_cm());
  if (!new_volume || !new_tone)
  {
    noToneAC();
  }
  else
  {
#if LOG_OUTPUT
    if (new_tone != cur_tone)
    {
      Serial.print("df=");
      Serial.println(abs(cur_tone - new_tone));
    }
#endif
    if (TONE_CHANGED_ENOUGH(new_tone, cur_tone) || new_volume != cur_volume)
    {
      toneAC(new_tone, new_volume, tone_duration, 1);
#if LOG_OUTPUT
      if (new_tone != cur_tone)
      {
        Serial.print("Frequency change from ");
        Serial.print(cur_tone);
        Serial.print(" to ");
        Serial.println(new_tone);
      }

      if (new_volume != cur_volume)
      {
        Serial.print("Volume change from ");
        Serial.print(cur_volume);
        Serial.print(" to ");
        Serial.println(new_volume);
      }
#endif

      cur_tone = new_tone;
      cur_volume = new_volume;
    }
  }
}
