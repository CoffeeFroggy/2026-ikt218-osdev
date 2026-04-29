# Assignment 5

## #Overview
This assignment implements a simple music player using the **PC speaker (PCSPK)** and the **Programmable Interval Timer (PIT)**.
Songs are played by generating frequencies and controlling timing between notes.

---

## #HowItWorks

### #PCSpeaker
The PC speaker is controlled through **port `0x61`**:
- Bits 0 and 1 enable/disable sound output

---

### #PIT
The PIT is used for:

#### #SoundGeneration
- Channel 2 generates square wave audio
- Frequency is set using:

divisor = PIT_BASE_FREQ / frequency


#### #Timing
- Channel 0 runs at ~1000 Hz
- A tick counter is incremented via interrupts
- `SleepInterrupt()` is used for accurate note timing

---

## #Implementation

### #Playback
Each note contains:
- Frequency (Hz)
- Duration (ms)

Playback works by:
1. Setting frequency with `PlaySound()`
2. Waiting using `SleepInterrupt()`
3. Stopping sound with `StopSound()`

Rests are handled using `R = 0`.

---

## #Challenges

### #Timing
Busy-wait delays caused incorrect timing.
This was solved by using interrupt-based sleeping.

### #QEMU
QEMU has limited PC speaker support:
- Fast note changes sound unclear
- Songs degrade at high speed

---

## #Adjustments

To improve playback clarity:

duration = original_duration * 2


This slows down the songs and makes notes more distinguishable.

---

## #Conclusion

The system successfully:
- Generates sound using the PC speaker
- Uses PIT for timing and frequency control
- Plays songs with multiple notes

Limitations in QEMU affect sound quality, but the implementation itself works as intended.
