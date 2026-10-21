# Winamp Classic Visualizer (VST3)

<img width="534" height="196" alt="image" src="https://github.com/user-attachments/assets/cb2ed132-e80e-492c-bedb-529d4f197312" />

The spectrum analyzer and oscilloscope from Winamp's classic main window, as a VST3 effect. The plugin passes audio through untouched and draws the 76 x 16 vis area at 1x to 10x.

## About

This plugin is inspired by Winamp. It is a complete redevelopment and does not redistribute any Winamp code. The Winamp source release was used only as a reference for how the classic vis behaves.

## Source reference

The reference is the Winamp source release, as mirrored at [alexfreud/winamp](https://github.com/alexfreud/winamp) (branch `community`, commit [`0954e03`](https://github.com/alexfreud/winamp/tree/0954e03b3acbc11b6c90298598db6b6469568d72)). These are the files used as reference:

| Original file | What it holds | Implemented in |
|---|---|---|
| [`Src/Winamp/SABuffer.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/SABuffer.cpp) | L + R sum, 512-sample Hann window, 256-sample step | `Source/Core/SpectrumAnalyzer.*` |
| [`Src/Winamp/classic_vis.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/classic_vis.cpp) | `makeSpecData` (75 bands) and `makeOscData` (scope points) | `Source/Core/SpectrumAnalyzer.*`, `Source/Core/Oscilloscope.*` |
| [`Src/Winamp/draw_sa.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/draw_sa.cpp) | `draw_sa`: bars, peaks, falloff, colorings, scope styles | `Source/Core/ClassicVis.*` |
| [`Src/Winamp/SA.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/SA.cpp) | The vis thread: refresh divider, 400 ms frame match | `Source/Core/ClassicVis.*`, `Source/Plugin/PluginEditor.cpp` |
| [`Src/Winamp/draw.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/draw.cpp) | Built-in vis palette and the `viscolor.txt` reader | `Source/Core/Palette.*` |
| [`Src/Winamp/config.h`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/config.h) | Default option values | `Source/Core/VisSettings.h` |
| [`Src/Winamp/options_classic.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/options_classic.cpp) | The options and their ranges | `Source/Plugin/Parameters.cpp` |
| [`Src/Winamp/FFT.cpp`](https://github.com/alexfreud/winamp/blob/0954e03b3acbc11b6c90298598db6b6469568d72/Src/Winamp/FFT.cpp) | `fft_9`, which calls `nsutil` (body not in the release) | `Source/Core/Fft.*` (written new) |

## What it reproduces

- **Analysis**: L + R summed into a 512-sample Hann window that moves 256 samples per frame. Then an unnormalised FFT, magnitude x 0.5, and 75 semitone-spaced bands from Winamp's `fpow2` warp with Hermite interpolation.
- **Drawing**: `draw_sa`'s integer bar falloff and peaks that fall faster each tick, on the same 32-row bottom-up bitmap. The bottom 2 rows of each bar are hidden, as in Winamp.
- **Timing**: the vis thread ticks at 70 Hz and draws every Nth tick, set by the refresh rate.
- **Scope**: the top byte of each 16-bit sample, divided by 8, at 75 points across a 576-sample block.

The one part not in the source release is the FFT itself (`nsutil`), so `Source/Core/Fft.cpp` is a plain radix-2 FFT with the same output layout.

## Options

Select **Settings** below the vis, or right-click the vis, for Winamp's menu. Left-click the vis to cycle the mode.

| Option | Values | Default |
|---|---|---|
| Mode | Disabled, Spectrum analyzer, Oscilloscope | Spectrum analyzer |
| Analyzer coloring | Normal, Fire, Line | Normal |
| Show peaks | on, off | on |
| Band width | Thick, Thin | Thick |
| Oscilloscope style | Dots, Lines, Solid | Lines |
| Refresh rate | 70, 35, 18, 9 fps | 35 fps |
| Analyzer falloff | Slowest to Fastest | Moderate |
| Peaks falloff | Slowest to Fastest | Slow |
| Colors | Load a skin's `viscolor.txt`, or the defaults | defaults |
| Size | 1x to 10x | 3x |

The defaults are Winamp's, from `config.h`. Every option except Colors and Size is a host parameter, so it saves with the project and can be automated.

## Build

GitHub Actions builds a universal macOS VST3 and a 64-bit Windows VST3. On each platform it runs the unit tests and `pluginval`, then uploads the zipped bundle as an artifact. To build by hand:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

The unit tests cover the core in `Source/Core` and need no JUCE. Their expected pixels come from tracing `draw_sa` by hand.

## Install

Download the ZIP for your platform from [GitHub Releases](https://github.com/alanmarcero/winamp-classic-visualizer-vst/releases/latest), then unzip it. Tagged releases publish only after unit tests and pluginval pass on both platforms.

### macOS

Move `Winamp Classic Visualizer.vst3` into `~/Library/Audio/Plug-Ins/VST3/`. The bundle is signed ad hoc, so clear the quarantine flag once:

```sh
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"Winamp Classic Visualizer.vst3"
```

### Windows

1. Right-click `WinampClassicVisualizer-VST3-Windows.zip`, select **Properties**, check **Unblock**, and select **OK**. Do this before you unzip, so the extracted files are not marked as downloaded.
2. Unzip it. You get a folder named `Winamp Classic Visualizer.vst3`.
3. Move that folder into `C:\Program Files\Common Files\VST3\`. Windows asks for administrator permission.
4. Rescan plugins in your DAW.

Only 64-bit DAWs can load it.
