# Summary of Work Done on April 27, 2026

## Overview

Today, work was done on the `OSDev_18` project to add a new piano application to the kernel. The main change was the introduction of a small PC-speaker piano mode that lets the user play notes from the keyboard, record note sequences, and replay recorded songs.

This work also required changes to the application menu in `kernel.c`, the build system, terminal input handling, and a few small adjustments to existing sound and game code so the new feature could fit into the current menu-driven OS flow.

## Git Evidence

The changes described in this note come from the newest local commit:

```text
7686d3e added piano feature
```

This commit is local work that has not been pushed yet.

## Piano Application

The largest part of the work was the addition of a new piano application:

- Added:
  ```c
  include/pianoApp/piano.h
  include/pianoApp/frequencies.h
  src/piano.c
  ```

The new piano code introduces:

- note frequency constants from `C4` up to `C5`
- a `Note` structure storing frequency and duration
- a `Song` structure storing a recorded note sequence
- a `SongLibrary` structure for keeping multiple saved songs
- a `PianoAppState` structure for tracking recording state, active note, and timing between notes

This gives the kernel a dedicated application module instead of placing all piano logic directly inside `kernel.c`.

## How the Piano Works

The piano uses the existing PIT and PC-speaker infrastructure already present in the kernel:

- `PianoPlaySound()` configures PIT channel 2 to generate the selected note frequency
- `PianoEnableSpeaker()` and `PianoDisableSpeaker()` control whether the sound is actually sent to the PC speaker
- `SleepInterrupt()` is used to control note timing

The key layout maps keyboard keys such as `z`, `x`, `c`, `v`, `b`, `n`, `m`, and `,` to white keys, while keys such as `s`, `d`, `g`, `h`, and `j` act as black keys.

The application also supports:

- `r` to start and stop recording
- `p` to play back a stored song
- `q` to quit the piano application

While recording, the code stores both notes and rests. Rests are detected by comparing the current PIT tick count with the time when the previous note ended. This means the playback system can preserve simple pauses between notes instead of only storing the frequencies themselves.

## Kernel and Menu Integration

The kernel menu in `src/kernel.c` was expanded so the user can launch the new application.

The menu now shows:

- `0. Play Music`
- `1. Play Snake`
- `2. Play Piano`

The kernel now includes `piano.h` and calls `PlayPiano()` when the user selects option `2`.

The menu flow was also cleaned up slightly:

- the terminal is cleared before entering each application
- the terminal is cleared again after returning from an application
- invalid input now prints an error, waits briefly using `SleepInterrupt(1000)`, and then clears the screen

This makes the menu loop more suitable for switching between multiple interactive kernel applications.

## Terminal and Input Support

To support the piano playback menu, terminal input handling was extended:

- Added to `terminal.h`:
  ```c
  int TerminalGetUInt(uint32_t *number);
  ```

- Implemented in `terminal.c`:
  ```c
  int TerminalGetUInt(uint32_t *number);
  ```

This helper reads a numeric value from keyboard input and converts it into an unsigned integer while checking for invalid characters and overflow.

It is used when the piano application asks which recorded song should be replayed.

Another visible input-related change was made in `keyboard.c`:

- `TerminalPutChar(ascii);`

was added inside the keyboard handler so typed characters are echoed to the terminal when key presses are processed.

## Related Adjustments to Existing Code

Some smaller changes were also made outside the new piano module:

- `src/songPlayer.c` no longer doubles note duration during playback, so it now uses the exact stored duration values
- `src/snake.c` now prints `Press q to exit` on the game screen
- `include/libc/limits.h` now defines `UINT32_MAX`, which is needed by `TerminalGetUInt()`

These are small changes, but they support the new interaction model and make the applications more consistent.

## Build System Update

The build configuration was updated so the new piano source is compiled into the kernel:

- Added to `CMakeLists.txt`:
  ```c
  src/piano.c
  ```

Without this change, the new application code would exist in the source tree but would not be linked into the final kernel binary.

## Current State

At the end of this work, the source tree contains a third kernel application alongside the existing music player and Snake game. The new code introduces:

- a keyboard-playable piano
- support for recording note sequences
- support for replaying saved songs
- a menu option for launching the piano from the main kernel loop

This note reflects the code changes present in the newest local commit. It describes the implementation work, but it does not by itself prove runtime verification in QEMU during this session.

## Later Debug and Audio Setup Work

After the piano feature was added, more work was done on the development setup so the new PC-speaker functionality could be tested through the existing VS Code and devcontainer workflow.

The first step was to compare a new QEMU command snippet against the current `AdvOpsys` setup. That check showed that the suggested command did not fit this machine and repository unchanged.

This was treated as a local environment compatibility issue, not as a problem in the `OSDev_18` source tree itself.

The main problems were:

- the suggested PulseAudio path `/mnt/wslg/PulseServer` is for WSLg, while this machine uses a Linux host Pulse socket instead
- the suggested `-s` flag exposes GDB on the default port `1234`, but the repository debug setup already uses port `26000`
- the repository boot flow expects `kernel.iso` as the CD image and `disk.iso` as a separate drive, so using `-hda` for `kernel.iso` does not match the current image layout

## VS Code Debug Script Changes

The existing QEMU launcher in `.vscode/qemu-debug.sh` was updated so it can choose an audio backend more carefully.

The script now:

- prefers PulseAudio when a usable Pulse socket or `PULSE_SERVER` value is available
- falls back to SDL audio when PulseAudio is not available
- keeps the existing `127.0.0.1:26000` GDB server path used by the VS Code debugger

This preserves the original debug flow while making the QEMU launch logic less tied to one audio backend.

## Devcontainer Mount Attempt and Revert

An attempt was also made to forward the Linux PulseAudio socket into the devcontainer through `.devcontainer/devcontainer.json`.

That change did not work with the current Docker Desktop `desktop-linux` environment. Rebuilding the container failed because Docker could not bind-mount the expected Pulse runtime directory:

```text
/run/user/1000/pulse
```

As a result, the PulseAudio mount and related container environment variable were removed again so the devcontainer could build and start normally.

This failure was specific to the local container/runtime setup on this machine. It did not indicate that the kernel project, the piano code, or the normal image build process were broken.

This means:

- the devcontainer build and debug workflow works again
- audio support cannot be relied on from the container itself in the same way

## Host-Side QEMU Workaround

Because the devcontainer could build and debug the kernel but still had no sound, a separate host-side QEMU workflow was added.

Two files were updated for this:

- `.vscode/launch.json`
- `.vscode/qemu-debug-host.sh`

The new host-side setup works by splitting the responsibilities:

- the kernel is still built from the existing VS Code/devcontainer environment
- QEMU is launched on the host, where it can access the real PulseAudio socket
- the debugger inside VS Code attaches to that host-side QEMU instance on port `26000`

This gives a more realistic path for testing PC-speaker audio on this machine, because the emulator process that generates sound now runs in the same environment as the actual audio server.

In other words, the remaining sound problem was due to where QEMU was running relative to the local audio server, not due to a fault in the repository code.

## Updated Current State

At the end of the day, the work on April 27 consisted of both feature development and environment debugging:

- a new piano application was added to the kernel
- the application menu and terminal input code were extended to support it
- the QEMU debug setup was reviewed against a new sound-enabled launch command
- the container-based PulseAudio mount attempt was tested and then rolled back because it broke devcontainer startup
- a host-side QEMU launch script and matching VS Code attach configuration were added as the current workaround for testing sound

So the codebase now contains the piano implementation itself, and the surrounding debug setup has also been adjusted to better support testing that feature on this specific machine.

The important distinction is that the main issue here was local system integration between Docker Desktop, the devcontainer, QEMU, and the host audio stack. It was not a core project issue in `AdvOpsys`.
