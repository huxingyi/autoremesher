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

## Install

Blender 4.2 or newer, as an extension (no legacy `bl_info`, so the name stays
in one place): zip the `autoremesher_blender` folder, then *Edit > Preferences >
Get Extensions > Install from Disk*. Then set the executable in the add-on's
preferences, e.g. `build/app/autoremesher`.

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
  machine this was written on, so `__init__.py` (operator, panel, preferences,
  mesh read and import), `blender_smoke.py` and the manifest have only been
  byte-compiled. Windows and macOS are untested (the Windows branch of
  `--stdin-control` has not been compiled).
