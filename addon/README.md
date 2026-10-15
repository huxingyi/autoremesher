# Quadami for Blender (minimal add-on)

Quad-remeshes the active mesh with the AutoRemesher core. Quadami is the
working name (2026-10-15) and may change before the first release: it lives in
`autoremesher_blender/names.py` plus `id`/`name` in `blender_manifest.toml`
(a test checks they agree). The core, its CLI and the repository keep the
name autoremesher. The remesher runs in
its own process, never inside Blender, so a crash or a stuck solve cannot take
Blender down.

License: GPL-3.0-or-later, like the rest of the fork.

## What it does

- **Sidebar > Quadami** shows the parameters of the core's `remesh()`
  (docs/INTERFACE.md §3) and nothing else: target quads, adaptivity,
  anisotropy, edge scaling, sharp edge angle, smooth normal angle. Ranges are
  the Qt app's. The default target is 5000 quads, not the CLI's 50000, so a
  first click on a laptop finishes quickly.
- **Remesh** reads the evaluated mesh (modifiers applied, object space), starts
  the worker and returns control to Blender. Progress shows in the status bar
  and the panel.
- **Esc** (or **Cancel** in the panel) asks the worker to stop. The core stops
  at its next cancel point, which can take a few seconds inside one solve
  (measured up to 3.5 s, INTERFACE §8), so the add-on waits 5 s before it kills
  the process.
- On success it adds `<name>_remesh` next to the source with the same
  transform and collections, hides the source and selects the result.
- The panel shows the quality report from the worker: status, quads against
  the target, non-quads, non-manifold edges, winding flips, folded faces, empty
  islands, time and warnings; non-zero defects are drawn in red. The report is
  kept on the result object (custom property `quadami_report`), so it is
  still there after saving the file. A failed or cancelled run shows its
  status and message instead; the worker's last output lines go to Blender's
  console.

## How the worker works (and why)

There is no Python binding of the C++ interface yet (TODO G1), so the worker
is the existing headless CLI, wrapped by `worker.py` (plain Python + numpy, no
`bpy`, so it is testable without Blender):

```
autoremesher -i input.obj -o output.obj --report-json report.json --stdin-control \
    --target-quads N --adaptivity A ...
```

| Need | How |
|---|---|
| separate process | `subprocess.Popen` of the CLI, `QT_QPA_PLATFORM=offscreen` |
| progress | the CLI's `42% done. <step>` lines on stdout, read by a thread |
| cancel | `cancel` line on stdin (new flag `--stdin-control`, see below); kill after 5 s |
| Blender dies | its end of the pipe closes, the worker reads EOF and cancels itself |
| result | the OBJ the CLI writes, plus `--report-json` (schema `autoremesher.remesh-result`) |
| version check | the report's `interface_version` must be 1.x |

`--stdin-control` is the one change to the C++ side: an opt-in flag that hands
a `CancelToken` to `remesh()` and cancels it when stdin gets `cancel` or is
closed. Without the flag the CLI behaves as before.

This differs from INTERFACE §4, which plans a Blender-Python worker hosting a
nanobind module and exchanging `.npz` files. Swapping in that worker later
only replaces `RemeshJob` in `worker.py`; the operator and panel only use its
`start / poll / cancel / done / outcome` methods. Costs of the CLI route that
the binding removes:

- OBJ text both ways (slow on multi-million-triangle inputs, REFERENCE P4).
- The CLI writes coordinates with 6 significant digits.
- The CLI is the Qt app: it needs Qt at run time and a QPA platform.
- The add-on needs the path to a built `autoremesher` (preferences, or
  `$AUTOREMESHER_EXECUTABLE`, or `autoremesher` on `PATH`). Nothing is bundled.

## Install (Linux, Blender 4.2 or newer)

The add-on is a Blender extension (4.2+ only; there is no legacy `bl_info`, so
the name stays in one place). The remesher binary has to be built on the same
machine and OS as the Blender that uses it.

1. **Blender.** Any Blender 4.2 or newer from blender.org (the Linux `.tar.xz`,
   extracted anywhere) or the Snap. Not the Flatpak: its sandbox cannot start
   a program from your home folder. On older GPUs (Intel before Kaby Lake,
   NVIDIA before GeForce 900, e.g. the E7450's HD 5500) use 4.5 LTS: Blender
   5.0 raised the minimum GPU to those generations
   (https://www.blender.org/download/requirements/).
2. **Zip and remesher**, from the repository (this checkout):

   ```
   sh addon/package.sh
   ```

   It prints two paths: the add-on zip (`build/addon/quadami-0.1.0.zip`) and
   the remesher binary (`build/app/autoremesher`). It builds the binary only
   if it is missing or too old (no `--stdin-control`), with `nice -n 10 make
   -j2` and no LTO, and refuses while another build runs. Build dependencies:
   see CLAUDE.md "Build" (Qt 5 dev packages, `libtbb-dev`).
3. **Install the zip.** Blender: *Edit > Preferences > Get Extensions*, the
   small down-arrow at the top right, *Install from Disk...*, pick the zip.
   (Dragging the zip onto the Blender window works too.)
4. **Point it at the remesher.** *Edit > Preferences > Add-ons*, expand
   *Quadami*, set *Remesher executable* to the binary path from step 2. The
   line below it must say *Using: ...* with a check mark. Alternatively start
   Blender with `AUTOREMESHER_EXECUTABLE=<path>` set, or put `autoremesher`
   on `PATH`.
5. **Open the panel.** In the 3D Viewport press `N`; the sidebar has a
   *Quadami* tab.

To update after a code change: run step 2 again, then in *Get Extensions*
remove Quadami and install the new zip (or restart Blender after reinstalling).

## First test in Blender

1. Start Blender from a terminal, so the worker's messages are visible there
   if something fails.
2. *Add > Mesh > Monkey*, then add a *Subdivision Surface* modifier at level 2
   (the add-on remeshes the mesh with modifiers applied). Stay in Object Mode.
3. In the *Quadami* tab set *Target Quads* to 2000 and press *Remesh*. The
   status bar shows the percentage and step; orbit the view to check that
   Blender stays responsive.
4. When it finishes, `Suzanne_remesh` appears, the source is hidden and the
   panel shows the report. Expect a status of *Done*, quads near the target,
   and red rows only for real defects (non-quads, non-manifold edges, winding
   flips, folded faces, empty islands).
5. `Ctrl+Z` once should remove the result and show the source again.
6. **Cancel:** select the source, set *Target Quads* to 50000 and press
   *Remesh*; after a few seconds press `Esc` (or *Cancel* in the panel). The
   status bar says *cancelling...*; within 5 s the panel shows *Cancelled* and
   no new object is added.
7. Optional, headless (no window):

   ```
   AUTOREMESHER_EXECUTABLE=build/app/autoremesher <blender> -b --factory-startup --python-exit-code 1 --python addon/tests/blender_smoke.py
   ```

   It should end with `blender_smoke: OK`.

If a step fails, copy the terminal output (lines after *Quadami worker
output*) and the panel's status line.

## Tests

Worker side, against a built CLI (no Blender needed):

```
AUTOREMESHER_EXECUTABLE=build/app/autoremesher python3 -m unittest discover -s addon/tests -v
```

Covers the manifest matching `names.py`, OBJ round trip, report rows, a remesh of a 16k-triangle sphere (counts
match the report, progress non-decreasing), cancel by `cancel` line and by
closed stdin (worker exits by itself and reports `cancelled`), kill after the
grace time, a rejected input and a worker that crashes without a report.

Inside Blender, headless (`blender_smoke.py`): reads a sphere through the
add-on, runs the worker and builds the result object, then checks counts and
transform. Background mode has no window, so the modal operator, the panel,
Esc and the progress display can only be checked by hand in the UI.

## Status (2026-10-15)

- Tested: everything in `test_worker.py`, on Ubuntu 24.04 with Python 3.12 and
  numpy 1.26, against the CLI built from this branch.
- **Not tested: anything that imports `bpy`.** Blender is not installed on the
  machine this was written on (searched the whole disk: no binary, AppImage,
  tarball, Snap, Flatpak or `~/.config/blender`), so `__init__.py` (operator, panel, preferences,
  mesh read and import), `blender_smoke.py` and the manifest have only been
  byte-compiled. Windows and macOS are untested (the Windows branch of
  `--stdin-control` has not been compiled).
