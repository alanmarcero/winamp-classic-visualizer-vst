# Winamp Classic Visualizer (VST3)

The spectrum analyzer and oscilloscope from Winamp's classic main window, as a VST3 effect. The plugin passes audio through untouched and draws the 76 x 16 vis area at 1x to 5x.

## Private use only

This code is ported from the Winamp source release (`Src/Winamp/draw_sa.cpp`, `SA.cpp`, `classic_vis.cpp`, `SABuffer.cpp`, `draw.cpp`, `config.h`), mirrored at [alexfreud/winamp](https://github.com/alexfreud/winamp). That code is under the Winamp Collaborative License 1.0, which allows changes for private use and forbids distribution of changed versions. Keep this repo private and do not share the built plugin.

## What is ported

- **Analysis**: L + R summed into a 512-sample Hann window that moves 256 samples per frame. Then an unnormalised FFT, magnitude x 0.5, and 75 semitone-spaced bands from Winamp's `fpow2` warp with Hermite interpolation.
- **Drawing**: `draw_sa`'s integer bar falloff and peaks that fall faster each tick, on the same 32-row bottom-up bitmap. The bottom 2 rows of each bar are hidden, as in Winamp.
- **Timing**: the vis thread ticks at 70 Hz and draws every Nth tick, set by the refresh rate.
- **Scope**: the top byte of each 16-bit sample, divided by 8, at 75 points across a 576-sample block.

The one part not in the source release is the FFT itself (`nsutil`), so `Source/Core/Fft.cpp` is a plain radix-2 FFT with the same output layout.

## Options

Right-click the vis for Winamp's menu. Left-click cycles the mode.

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
| Size | 1x to 5x | 3x |

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

Download the artifact for your platform from the latest run on the repo's Actions page. GitHub wraps each artifact in a zip, so unzip it once to get the plugin's zip, then unzip that.

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
