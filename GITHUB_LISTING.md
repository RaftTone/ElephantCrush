# Ready-to-paste text for the GitHub repo

## Repo name
`ElephantCrush`

## About (short description — the field at the top of the repo page)
> Free native Apple-Silicon distortion plugin (AU/VST3) — sounds like Camel Audio's classic CamelCrusher, rebuilt from scratch for arm64. Not affiliated with Apple/Camel Audio.

## Topics / tags (add these in the repo "Topics" box)
`audio-plugin` `vst3` `audio-unit` `juce` `macos` `apple-silicon` `arm64` `distortion`
`audio-effect` `music-production` `camelcrusher` `logic-pro` `cpp`

---

## First release — title
`ElephantCrush 1.0.0 — native Apple Silicon`

## First release — notes
```
ElephantCrush 1.0.0

A free, native Apple Silicon (arm64) distortion / colour plugin for macOS (AU + VST3).
It recreates the sound and workflow of Camel Audio's classic free CamelCrusher — which is
Intel-only and won't run natively on Apple Silicon — as an independent, from-scratch build.

• Native arm64 (also universal x86_64) — no Rosetta
• Dual distortion, resonant low-pass filter, compressor (with PHAT mode), wet/dry master
• 20 presets + randomiser
• Loads in Logic Pro, Ableton Live, and other AU/VST3 hosts

INSTALL (no building):
1. Download ElephantCrush-macOS.zip below.
2. Unzip and drag ElephantCrush.component into ~/Library/Audio/Plug-Ins/Components
   and ElephantCrush.vst3 into ~/Library/Audio/Plug-Ins/VST3.
3. Restart your DAW.

If macOS says it's "damaged"/can't be opened (it's just unsigned, not notarized), run in Terminal:
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/ElephantCrush.component
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ElephantCrush.vst3

Independent project — not affiliated with, endorsed by, or connected to Apple or Camel Audio.
No original code, graphics, names, or presets are included.
```

**Attach to the release:** the file `ElephantCrush-macOS.zip` (already generated in the project
folder — it contains both the `.component` and `.vst3`). Drag it onto the release's "Attach
binaries" area on GitHub.

---

## Short announcement blurb (forums / Reddit / social)
> Made a free, native Apple-Silicon distortion plugin that sounds like the old CamelCrusher
> (which is Intel-only and won't run natively on M-series Macs). Rebuilt from scratch in JUCE,
> AU + VST3, open source. Not affiliated with Apple/Camel Audio. Link + code: <your repo URL>
